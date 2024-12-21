#pragma once
#ifndef _MARCH_RENDERER_H
#define _MARCH_RENDERER_H

#include <vector>
#include <thread>

struct RenderInfo;

// represents some information related to March renderer's options
struct MarchOpts {
    float max_dist;
    size_t max_iter;
    float intersect_threshold;
};

class MarchRenderer {
public:
    MarchRenderer(MarchOpts *profile_ptr);

    void render(RenderInfo *info_ptr, size_t thread_count);

    float get_max_dist() const;
    size_t get_max_iter() const;
    float get_intersect_threshold() const;

private:
    std::vector<std::thread> _threads;
    MarchOpts* _opts_ptr;
};


#endif // _MARCH_RENDERER_H
