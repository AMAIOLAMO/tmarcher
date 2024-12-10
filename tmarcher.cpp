#include <cfloat>
#include <cstring>
#include <assert.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include <unistd.h>

// TODO: replace this with a submodule from git instead of copying versions in a manual way
#define CXVEC_IMPL_ONCE
#include <cxvec.h>

#include <camera.h>
#include <term_util.h>
#include <texture.h>
#include <math_util.h>
#include <noise_utils.h>
#include <key_input.h>
#include <rotors.h>
#include <sdfable.h>
#include <material.h>
#include <primitives.h>
#include <renderable.h>

// __obsolete__
/*char getc_from_brightness(float brightness) {*/
/*    const char *characters = " .~;*=!O#$NM@";*/
/*    const size_t len = std::strlen(characters);*/
/**/
/*    if(brightness > 1)*/
/*        return characters[len - 1];*/
/**/
/*    if(brightness < 0)*/
/*        return characters[0];*/
/**/
/*    return characters[(size_t)(brightness * (len - 1))];*/
/*}*/

using namespace std::chrono;

typedef steady_clock::time_point time_point_t;

steady_clock::time_point get_time_now() {
    return steady_clock::now();
}

#define S2MS 1000.0

double get_delta(steady_clock::time_point previous_time) {
    return duration_cast<duration<double>>(get_time_now() - previous_time).count();
}


//calculates the ray direction utilizing basic vector mathematics / analysis
Vec3f ray_dir_from_uv(Camera *cam_ptr, Vec2f uv) {
    // this calculates the offset on the camera screen frustrum plane
    Vec3f offset_right = cam_ptr->right() * uv.x;
    Vec3f offset_up = cam_ptr->up() * uv.y;

    // then finds it's point in space relative to the camera position
    Vec3f screen_point = cam_ptr->screen_center() + offset_right + offset_up;

    // then calculates the ray direction, do remember to normalize the length (since it's considered as a direction)
    return Vec3f_norm(screen_point - cam_ptr->pos);
}

// NOT USED
/*void swapf(float *x, float *y) {*/
/*    float t = *x;*/
/*    *x = *y;*/
/*    *y = t;*/
/*}*/

Renderable* scene_pt_renderable(const std::vector<Renderable*> &renderables, Vec3f pt) {
    if(renderables.size() == 0)
        return nullptr;
    // else

    Renderable *closest = renderables[0];

    for(size_t i = 1; i < renderables.size(); i++) {
        if(renderables[i]->dist(pt) >= closest->dist(pt))
            continue;

        closest = renderables[i];
    }

    return closest;
}

Vec3f normal_from_pt_approx(SDFable *sdfable_ptr, Vec3f pt) {
    Vec2f e = Vec2f{.001, 0.};

    float d = sdfable_ptr->dist(pt);

    Vec3f normal = {
            d - sdfable_ptr->dist(pt - Vec3f{e.x, e.x, e.y}),
            d - sdfable_ptr->dist(pt - Vec3f{e.y, e.x, e.y}),
            d - sdfable_ptr->dist(pt - Vec3f{e.y, e.y, e.x})
        };

    return Vec3f_norm(normal);
}

Vec3f normal_from_pt_backtrack(Vec3f rd, SDFable *sdfable_ptr, Vec3f pt) {
    Vec2f e = Vec2f{.001, 0.};

    pt = pt - rd * e.x;

    float d = sdfable_ptr->dist(pt);

    Vec3f normal = {
            d - sdfable_ptr->dist(pt - Vec3f{e.x, e.x, e.y}),
            d - sdfable_ptr->dist(pt - Vec3f{e.y, e.x, e.y}),
            d - sdfable_ptr->dist(pt - Vec3f{e.y, e.y, e.x})
        };

    return Vec3f_norm(normal);
}


// calculates the normal of the surface of an object based on an intersecting point
Vec3f normal_from_pt_approx(std::vector<Renderable*> &renderables, Vec3f pt) {
    Renderable *r = scene_pt_renderable(renderables, pt);
    return normal_from_pt_approx(r, pt);
}

#define MAX_MARCHDIST 300
#define MAX_MARCH_ITER 100

#define MARCH_INTERSECT_THRESHOLD .003

struct HitInfo {
    Vec3f pt;
    float dist;
    float min_penumbra;
    int iter;
    Renderable *renderable_ptr;

    bool is_hit() const {
        return dist < MAX_MARCHDIST;
    }

    void set_hit_failed() {
        dist = MAX_MARCHDIST;
    }
};

// this is the main function in which handles how to trace a ray from the ray origin to scene utilizing scene distance fields
// TODO: THIS TAKES A LOT OF TIME, maybe we can utilize data structuresh(grid based / tree based) to improve this?
HitInfo raymarch(const std::vector<Renderable*> &renderables,
                 Vec3f ro, Vec3f rd, float penumbra_factor = 8) {
    HitInfo info;
    info.dist = 0;
    info.min_penumbra = 1.0;

    // loop and march forward given the scene distance, until the closest march distance is very tiny, then it's considered a hit
    // and breaks out of loop if the distance marched is too far (considered out of render distance)
    for(info.iter = 0; info.iter < MAX_MARCH_ITER; ++info.iter) {
        info.pt = ro + rd * info.dist;

        Renderable *closest_renderable_ptr = scene_pt_renderable(renderables, info.pt);

        if(closest_renderable_ptr == nullptr) {
            info.set_hit_failed();
            return info;
        }

        info.renderable_ptr = closest_renderable_ptr;

        float march_dist = closest_renderable_ptr->dist(info.pt);

        // free softshadow penumbra
        info.min_penumbra = minf(info.min_penumbra, penumbra_factor * march_dist / info.dist);

        info.dist += march_dist;

        if(march_dist < MARCH_INTERSECT_THRESHOLD || info.dist > MAX_MARCHDIST) break;
    }

    return info;
}

#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 50
#define DEFAULT_THREAD_COUNT 1

struct RenderInfo {
    int iwidth, iheight;
    std::vector<Renderable*> *renderables_ptr;

    float *u_time_ptr;
    float *delta_ptr;

    Camera *cam_ptr;
    ByteColor *frame_buffer_ptr;
};

void render_quadrant_thread(int start_y, int end_y, RenderInfo *render_info_ptr) {
    int iwidth  = render_info_ptr->iwidth;
    int iheight = render_info_ptr->iheight;

    Camera *cam_ptr = render_info_ptr->cam_ptr;

    std::vector<Renderable*> *renderables_ptr = render_info_ptr->renderables_ptr;
    float *u_time_ptr = render_info_ptr->u_time_ptr;
    ByteColor *frame_buffer_ptr = render_info_ptr->frame_buffer_ptr;

    float width_height_ratio = (float)iwidth / iheight;

    const Vec3f light = {5.f, 20.f, 20.0f};

    for(int y = start_y; y <= end_y; ++y) {
        for(int x = 0; x < iwidth; ++x) {
            /*auto prepare_time_mark = get_time_now();*/
            // calculates the x and y percentage, commonly called uv / st in shaders
            Vec2f uv{
                (float)x / (iwidth - 1), (float)y / (iheight - 1)
            };

            // flip vertically, zero zero bottom left, because ascii draws from top left to bottom right
            uv.y = 1.0 - uv.y;

            // make center and scale to -1 ~ 1
            uv.x -= .5;
            uv.y -= .5;

            // adjusts the percentage on width be based on the aspect ratio (so that the screen will remain the same)
            // the height is keeped
            uv.x *= width_height_ratio;

            // calculates the ray direction shooting out from the camera position through the frustrum screen
            Vec3f rd = ray_dir_from_uv(cam_ptr, uv);

            /*auto cam_time_mark = get_time_now();*/
            HitInfo cam_hit = raymarch(*renderables_ptr, cam_ptr->pos, rd);

            Renderable *renderable = cam_hit.renderable_ptr;

            // TODO: ambient can be stored within a setting
            ByteColor ambient = lerp_color({111, 115, 210}, {163, 213, 255}, saturatef(uv.y));

            float vignette = 1.0 - saturatef(Vec2f_lengthf(uv) - .5f);

            ByteColor result_color;

            // if we hit anything (in other words, our distance is less than our maximum ray marching distance)
            if(cam_hit.is_hit()) {
                // utilizes phong shading, here we calculate the basic properties of the surface to be used
                Vec3f hit_to_light = Vec3f_norm(light - cam_hit.pt);
                Vec3f normal = normal_from_pt_backtrack(rd, cam_hit.renderable_ptr, cam_hit.pt);

                Vec3f cam_to_hit = Vec3f_norm(cam_hit.pt - cam_ptr->pos);
                float cam_to_hit_dist = Vec3f_lengthf(cam_hit.pt - cam_ptr->pos);
                cam_to_hit = Vec3f_scale(cam_to_hit, 1.0f / cam_to_hit_dist);

                Vec3f reflect = Vec3f_norm(Vec3f_mirror(hit_to_light, normal));

                HitInfo light_hit = raymarch(*renderables_ptr, cam_hit.pt + hit_to_light * .2f, hit_to_light, 16);

                float fog_strength = powf(minf(cam_to_hit_dist / MAX_MARCHDIST, MAX_MARCHDIST), 3.0f);

                // then calculate the basic components of a phong shading:
                // ambient + diffuse + specular = phong shading
                // we also add 1 and scale by half, because we dont want it to be negative value
                float diffuse = (Vec3f_dot(hit_to_light, normal) + 1.) * .5f;
                float specular = powf((Vec3f_dot(cam_to_hit, reflect) + 1.) * .5f, 50.0f);

                float occlusion = saturatef(1.0 - (float)cam_hit.iter / (float)MAX_MARCH_ITER);
                occlusion = lerpf(occlusion, 1.0, cam_to_hit_dist / MAX_MARCHDIST);

                // the calculation of the color, simply is the combination of all colours with respect of it's phong shading
                result_color = ambient * .3f + renderable->frag(cam_hit.pt, *u_time_ptr) * .3f +
                    renderable->frag(cam_hit.pt, *u_time_ptr) * diffuse + ByteColor::White() * specular;

                result_color = result_color * occlusion;

                float shadow = maxf(light_hit.min_penumbra, .3); // ensure the shadow mask isnt too dark
                result_color = result_color * shadow;

                result_color = lerp_color(result_color, ambient, fog_strength);
            }
            else
                result_color = ambient * .9f;

            result_color = result_color * vignette;

            frame_buffer_ptr[y * iwidth + x] = result_color;
        }
    }
}

void get_key_thread(int *key_ptr) {
    while(true) {
        *key_ptr = key_press();
    }
}

void tmarcher_dispatch_render_frame(RenderInfo *render_info_ptr,
                                    std::vector<std::thread> *render_threads_ptr, int thread_count) {
    int iheight = render_info_ptr->iheight;

    int quadrant_height = iheight / thread_count;
    render_threads_ptr->clear();

    for(int q = 0; q < thread_count - 1; q++) {
        int sy = q * quadrant_height;
        int ey = (q + 1) * quadrant_height - 1;

        render_threads_ptr->emplace_back(
            render_quadrant_thread, sy, ey, render_info_ptr
        );
    }

    render_threads_ptr->emplace_back(
        render_quadrant_thread, (thread_count - 1) * quadrant_height, iheight - 1, render_info_ptr
    );
}

Texture crate_texture;

int main() {
    // TODO: create an FPS locker, based on the current FPS value
    // TODO: write a resource loader that all materials will utilize instead of hardcode global variable
    crate_texture = texture_load_file("vendor/crate.ppm");

    for(int i = 0; i <= 20; ++i) printf(i == 20 ? "\n" : "----");
    printf("\tTerminal Raymarch Renderer by CxRedix v 0.1\n");
    for(int i = 0; i <= 20; ++i) printf(i == 20 ? "\n" : "----");
    
    int iwidth;
    int iheight;
    int thread_count;

    // fancy way of doing a simple thing, ugly. dont do this. (TODO)
    printf("Please input width(width > 0, otherwise default to %d):", DEFAULT_WIDTH);
    if(try_input_int(&iwidth, [](int v) { return v > 0; }) == false)
        iwidth = DEFAULT_WIDTH; 

    printf("Please input height(height > 0, otherwise default to %d):", DEFAULT_HEIGHT);
    if(try_input_int(&iheight, [](int v) { return v > 0; }) == false)
        iheight = DEFAULT_HEIGHT; 

    printf("Please input thread count(thread count > 0, otherwise default to %d):", DEFAULT_THREAD_COUNT);
    if(try_input_int(&thread_count, [](int v) { return v > 0; }) == false)
        thread_count = DEFAULT_THREAD_COUNT; 

    // setup virtual buffer for console
    // TODO: profile this before even using this, I am not sure how much this even affects performance
    // since it doesnt seem like a good trade off
    /*size_t vbuffer_size = 2 * iwidth * iheight;*/
    /*char *vbuffer = (char*)malloc(vbuffer_size);*/
    /*setvbuf(stdout, vbuffer, _IOFBF, vbuffer_size);*/

    auto frame_time = get_time_now();

    Camera cam{
        .pos     = {0, 10, -50},
        .look_at = {0, 0, 0 },
        .zoom    = 1.0f
    };

    std::vector<Renderable*> renderables;

    // construction of the renderables
    SolidMaterial red{ByteColor(255, 0, 0)};
    SolidMaterial green{ByteColor(0, 255, 0)};
    SolidMaterial white{ByteColor(255, 255, 255)};

    FuncMaterial terrain_material(
        [](SDFable *sdfable_ptr, Vec3f world_pt, float time) {
            (void) sdfable_ptr;
            (void) time;
            return ByteColor(saturatef(fBM_perlin2f(world_pt.x * 0.01, world_pt.z * 0.01, 3) * .5 + .5));
        }
    );

    Cube cube {
        {0, 0, 0}, {10, 10, 10}
    };

    Sphere sphere {
        {10, 30, 10}, 4
    };

    Renderable sphere_renderable {
        &sphere, &green
    };

    Vec3f sphere_velocity{0, 0, 0};

    Renderable cube_renderable {
        &cube, &white
    };

    Plane plane {
        -10
    };

    Renderable plane_renderable {
        &plane, &terrain_material
    };


    renderables.emplace_back(&plane_renderable);
    renderables.emplace_back(&sphere_renderable);

    float u_time = 0;

    // actual infinite loop for rendering each frame
    clear_screen();
    move_cursor(0, 0);

    ByteColor *frame_buffer = (ByteColor*)malloc(sizeof(ByteColor) * iwidth * iheight);

    std::vector<std::thread> render_threads;

    int key_input;

    std::thread input_thread(get_key_thread, &key_input);

    float rot_horizontal = 0;
    float rot_vertical = 0;

    Vec3f look_dir;
    Versor rot = versor_identity();

    for(;;) {
        // PROFILE TIMERS
        double tot_stdout_time = 0;
        double tot_pixel_time = 0;

        auto frame_render_time_mark = get_time_now();

        float delta = get_delta(frame_time);
        frame_time = get_time_now();
        u_time += delta;

        float rot_speed = 0.1;

        // SIMPLE CAMERA MOVEMENT
        if(key_input == 'h')
            rot_horizontal -= rot_speed;

        if(key_input == 'l')
            rot_horizontal += rot_speed;

        if(key_input == 'k')
            rot_vertical -= rot_speed;

        if(key_input == 'j')
            rot_vertical += rot_speed;

        rot_vertical = clampf(rot_vertical, -PI * 0.33333f, PI * 0.33333f);

        look_dir = {
            0, 0, 1.0
        };

        rot = versor_from_axis_rot({0, 1, 0}, rot_horizontal);
        rot = versor_mul(versor_from_axis_rot(cam.right(), rot_vertical), rot);

        look_dir = versor_rot3f(rot, look_dir);

        cam.look_at = cam.pos + look_dir;


        Vec2i input_strength{0, 0};

        if(key_input == 'w')
            input_strength.y += 1;

        if(key_input == 's')
            input_strength.y -= 1;

        if(key_input == 'a')
            input_strength.x -= 1;

        if(key_input == 'd')
            input_strength.x += 1;


        cam.move(
            cam.right() * input_strength.x * delta * 50.0f +
            cam.forward() * input_strength.y * delta * 50.0f
        );

        // SPHERE PHYSICS, TODO: make a physics state solver

        if(key_input == 'g')
            sphere_velocity = sphere_velocity + cam.forward() * 100.0f;

        if(key_input == ' ')
            sphere_velocity = sphere_velocity + (Vec3f){0, 1000.0f, 0};
        
        sphere_velocity.y += -9.18f;

        sphere_velocity = sphere_velocity * .95f;

        sphere.pos = sphere.pos + sphere_velocity * delta;

        if((sphere.pos.y - sphere.radius) < plane.y) {
            sphere.pos.y = plane.y + sphere.radius;
            sphere_velocity.y = -sphere_velocity.y * .8f;
        }


        key_input = EOF;

        auto pixel_time_mark = get_time_now();

        RenderInfo render_info{
            iwidth, iheight, &renderables, &u_time, &delta, &cam, frame_buffer
        };

        tmarcher_dispatch_render_frame(&render_info, &render_threads, thread_count);

        // wait until render complete
        for(std::thread &render_thread : render_threads)
            render_thread.join();
        // frame render complete

        tot_pixel_time += get_delta(pixel_time_mark);

        auto print_time_mark = get_time_now();

        move_cursor(0, 0);
        for(int y = 0; y < iheight; y++) {
            for(int x = 0; x < iwidth; x++) {
                ByteColor color = frame_buffer[y * iwidth + x];
                set_fg_color(color);

                fwrite("██", sizeof("██"), 1, stdout);
            }
            fwrite("\n", sizeof(char), 1, stdout);
        }

        tot_stdout_time += get_delta(print_time_mark);

        reset_color();

        double frame_render_time_sec = get_delta(frame_render_time_mark);

        // TODO: create a class to abstract and automatically handle profiling and debug information
        printf("============ PROFILE =============");
        printf("\nthread count: %d\n", thread_count);
        printf("\ntime: %.2fs", u_time);
        printf("\nFPS: %.2f", 1.0f / delta);
        printf("\nframe render: %.2lfms", frame_render_time_sec * S2MS);
            printf("\n\t> pixel render: %.2lfms (%.2lf%%)",   tot_pixel_time * S2MS,    (tot_pixel_time / frame_render_time_sec) * 100.0);

            printf("\n\t> total stdout: %.2lfms (%.2lf%%)",   tot_stdout_time * S2MS,   (tot_stdout_time / frame_render_time_sec) * 100.0);
        printf("\n==============================");

        printf("\n look dir: <%.2f, %.2f, %.2f>", look_dir.x, look_dir.y, look_dir.z);

        printf("\n rot quat: <%.2f, %.2f, %.2f, %.2f>", rot.w, rot.i, rot.j, rot.k);
        printf("\n rot len: %.2f", versor_len(rot));
        printf("\n cam right:");
        Vec3f_fprint(stdout, cam.right());
        printf("\n Rot horizontal: %f", rot_horizontal);
        printf("\n Rot vertical: %f", rot_vertical);
        printf("\n sphere velocity: <%.2f, %.2f, %.2f>", sphere_velocity.x, sphere_velocity.y, sphere_velocity.z);
    }

    /*free(vbuffer);*/
    free(frame_buffer);

    // unload everything
    unload_texture(crate_texture);

    return 0;
}

