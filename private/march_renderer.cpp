#include "../public/march_renderer.h"
#include "../public/renderable.h"
#include "../public/camera.h"
#include "../public/hitinfo.h"
#include "../public/math_util.h"
#include "../public/term_util.h"
#include "../public/renderinfo.h"

#include <vector>

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

// this is the main function in which handles how to trace a ray from the ray origin to scene utilizing scene distance fields
// TODO: THIS TAKES A LOT OF TIME, maybe we can utilize data structuresh(grid based / tree based) to improve this?
HitInfo raymarch(const std::vector<Renderable*> &renderables, MarchOpts *opts_ptr,
                 Vec3f ro, Vec3f rd, float penumbra_factor = 8) {
    HitInfo info;
    info.hit = false;
    info.dist = 0;
    info.min_penumbra = 1.0;

    // no renderables, just return failed to hit
    if(renderables.size() <= 0)
        return info;

    // loop and march forward given the scene distance, until the closest march distance is very tiny, then it's considered a hit
    // and breaks out of loop if the distance marched is too far (considered out of render distance)
    for(info.iter = 0; info.iter < opts_ptr->max_iter; ++info.iter) {
        info.pt = ro + rd * info.dist;

        Renderable *closest_renderable_ptr = scene_pt_renderable(renderables, info.pt);

        info.renderable_ptr = closest_renderable_ptr;

        float march_dist = closest_renderable_ptr->dist(info.pt);

        // free soft shadow penumbra
        info.min_penumbra = minf(info.min_penumbra, penumbra_factor * march_dist / info.dist);

        info.dist += march_dist;
        
        // hit since it's considered as intersect
        if(march_dist < opts_ptr->intersect_threshold) {
            info.hit = true;
            break;
        }

        // too far, consider no hit
        if(info.dist > opts_ptr->max_dist)
            break;
    }

    return info;
}



void render_quadrant_thread(int start_y, int end_y, RenderInfo *render_info_ptr, MarchOpts *opts_ptr) {
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
            uv = uv - Vec2f_make(.5);

            // adjusts the percentage on width be based on the aspect ratio (so that the screen will remain the same)
            // the height is keeped
            uv.x *= width_height_ratio;

            // calculates the ray direction shooting out from the camera position through the frustrum screen
            Vec3f rd = ray_dir_from_uv(cam_ptr, uv);

            /*auto cam_time_mark = get_time_now();*/
            HitInfo cam_hit = raymarch(*renderables_ptr, opts_ptr, cam_ptr->pos, rd);

            Renderable *renderable = cam_hit.renderable_ptr;

            // TODO: ambient can be stored within a setting
            ByteColor ambient = lerp_color({111, 115, 210}, {163, 213, 255}, saturatef(uv.y));

            float vignette = 1.0 - saturatef(Vec2f_lengthf(uv) - .5f);

            ByteColor result_color;

            // if we hit anything (in other words, our distance is less than our maximum ray marching distance)
            if(cam_hit.hit) {
                // utilizes phong shading, here we calculate the basic properties of the surface to be used
                Vec3f hit_to_light = Vec3f_norm(light - cam_hit.pt);
                Vec3f normal = normal_from_pt_backtrack(rd, cam_hit.renderable_ptr, cam_hit.pt);

                Vec3f cam_to_hit = Vec3f_norm(cam_hit.pt - cam_ptr->pos);
                float cam_to_hit_dist = Vec3f_lengthf(cam_hit.pt - cam_ptr->pos);
                cam_to_hit = Vec3f_scale(cam_to_hit, 1.0f / cam_to_hit_dist);

                Vec3f reflect = Vec3f_norm(Vec3f_mirror(hit_to_light, normal));

                HitInfo light_hit = raymarch(*renderables_ptr, opts_ptr, cam_hit.pt + hit_to_light * .2f, hit_to_light, 16);

                float fog_strength = powf(minf(cam_to_hit_dist / opts_ptr->max_dist, opts_ptr->max_dist), 3.0f);

                // then calculate the basic components of a phong shading:
                // ambient + diffuse + specular = phong shading
                // we also add 1 and scale by half, because we dont want it to be negative value
                float diffuse = (Vec3f_dot(hit_to_light, normal) + 1.) * .5f;
                float specular = powf((Vec3f_dot(cam_to_hit, reflect) + 1.) * .5f, 50.0f);

                float occlusion = saturatef(1.0 - (float)cam_hit.iter / (float)opts_ptr->max_iter);
                occlusion = lerpf(occlusion, 1.0, cam_to_hit_dist / opts_ptr->max_dist);

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

MarchRenderer::MarchRenderer(MarchOpts *opts_ptr) : _opts_ptr(opts_ptr) {
}

void MarchRenderer::render(RenderInfo *info_ptr, size_t thread_count) {
    int iheight = info_ptr->iheight;

    int quadrant_height = iheight / thread_count;
    _threads.clear();

    for(size_t q = 0; q < thread_count - 1; q++) {
        int sy = q * quadrant_height;
        int ey = (q + 1) * quadrant_height - 1;

        _threads.emplace_back(
            render_quadrant_thread, sy, ey, info_ptr, _opts_ptr
        );
    }

    _threads.emplace_back(
        render_quadrant_thread, (thread_count - 1) * quadrant_height, iheight - 1, info_ptr, _opts_ptr
    );

    for(std::thread &render_thread : _threads)
        render_thread.join();
}

