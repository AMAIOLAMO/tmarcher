#include <cfloat>
#include <cstring>
#include <assert.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include <unistd.h>

#define CXVEC_IMPL_ONCE
#include <cxvec.h>

#include <camera.h>
#include <term_util.h>
#include <texture.h>
#include <math_util.h>
#include <noise_util.h>
#include <key_input.h>
#include <versor.h>
#include <sdfable.h>
#include <material.h>
#include <primitives.h>
#include <time_util.h>
#include <renderinfo.h>
#include <march_renderer.h>
#include <renderable.h>
#include <term_displayer.h>

#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 50
#define DEFAULT_THREAD_COUNT 1

struct TimeMark {
    TimeMark(double *time_add_ptr) :
        time_add_ptr(time_add_ptr), start_mark(time_mark_now()) {
    }

    ~TimeMark() {
        *time_add_ptr += get_delta(start_mark);
    }

    double *time_add_ptr;
    time_mark_t start_mark;
};

void get_key_thread(int *key_ptr) {
    while(true) {
        *key_ptr = key_press();
    }
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
    ask_int_or("Please input width(width > 0)",
               &iwidth, DEFAULT_WIDTH, [](int v) { return v > 0; });

    ask_int_or("Please input height(height > 0)",
               &iheight, DEFAULT_HEIGHT, [](int v) { return v > 0; });

    ask_int_or("Please input thread count(thread count > 0)",
               &thread_count, DEFAULT_THREAD_COUNT, [](int v) { return v > 0; });

    clear_screen();

    // setup virtual buffer for console
    // TODO: profile this before even using this, I am not sure how much this even affects performance
    // since it doesnt seem like a good trade off
    /*size_t vbuffer_size = 2 * iwidth * iheight;*/
    /*char *vbuffer = (char*)malloc(vbuffer_size);*/
    /*setvbuf(stdout, vbuffer, _IOFBF, vbuffer_size);*/

    auto frame_time = time_mark_now();

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

    MarchOpts march_opts {
        300, 100, .003
    };

    MarchRenderer renderer {
        &march_opts
    };

    TerminalDisplayer displayer {
    };

    ByteColor *frame_buffer = new ByteColor[iwidth * iheight];

    int key_input;

    std::thread input_thread(get_key_thread, &key_input);

    float rot_horizontal = 0;
    float rot_vertical = 0;

    Vec3f look_dir;

    for(;;) {
        // PROFILE TIMERS
        auto frame_render_time_mark = time_mark_now();

        float delta = get_delta(frame_time);
        frame_time = time_mark_now();
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

        // we do not want to rotate exactly 180, because this will cause some calculations to divide by zero, which 
        // is not what we want
        // HACK: the above is prob a thing to account for, the one below is NOT a fix, but a HACK
        rot_vertical = clampf(rot_vertical, -PI * 0.33333f, PI * 0.33333f);

        look_dir = {
            0, 0, 1.0
        };

        Versor rot = versor_from_axis_rot({0, 1, 0}, rot_horizontal);

        // do NOT use the cam.right() here, because it was not recalculated after rotation from the one on the top yet
        Vec3f cam_right = versor_rot3f(rot, {1, 0, 0});

        rot = versor_mul(versor_from_axis_rot(cam_right, rot_vertical), rot);

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

        // TODO: make a physics state solver, probably using simplectic euler
        // probably also utilizing acceleration in this case

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
        
        

        double tot_pixel_time = 0;

        // rendering
        {
            TimeMark _{&tot_pixel_time};

            RenderInfo render_info{
                iwidth, iheight, &renderables, &u_time, &delta, &cam, frame_buffer
            };

            renderer.render(&render_info, thread_count);
        }

        double tot_stdout_time = 0;

        // display
        {
            TimeMark _{&tot_stdout_time};
            displayer.display(frame_buffer, iwidth, iheight);
        }

        reset_color();

        double frame_render_time_sec = get_delta(frame_render_time_mark);

        // TODO: create a class to abstract and automatically handle profiling and debug information
        printf("\n============ PROFILE =============");
        printf("\nthread count: %d\n", thread_count);
        printf("\ntime: %.2fs", u_time);
        printf("\nFPS: %.2f", 1.0f / delta);
        printf("\nframe render: %.2lfms", frame_render_time_sec * S2MS);
            printf("\n\t> pixel render: %.2lfms (%.2lf%%)",   tot_pixel_time * S2MS,    (tot_pixel_time / frame_render_time_sec) * 100.0);

            printf("\n\t> total stdout: %.2lfms (%.2lf%%)",   tot_stdout_time * S2MS,   (tot_stdout_time / frame_render_time_sec) * 100.0);
        printf("\n==============================");

        printf("\n============ INFO =============");
        printf("\n look dir: ");
        Vec3f_fprint(stdout, look_dir);

        printf("\n Rot horizontal: %f", rot_horizontal);
        printf("\n Rot vertical: %f", rot_vertical);

        printf("\n cam right:");
        Vec3f_fprint(stdout, cam.right());

        printf("\n sphere velocity: ");
        Vec3f_fprint(stdout, sphere_velocity);
        printf("\n==============================");
    }

    delete[] frame_buffer;

    // unload everything
    unload_texture(crate_texture);


    return 0;
}

