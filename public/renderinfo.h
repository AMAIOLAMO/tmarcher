#pragma once
#ifndef _RENDERINFO_H
#define _RENDERINFO_H

#include <vector>

struct Renderable;
struct Camera;
struct ByteColor;
struct MarchOpts;

struct RenderInfo {
    int iwidth, iheight;
    std::vector<Renderable*> *renderables_ptr;

    float *u_time_ptr;
    float *delta_ptr;

    Camera *cam_ptr;
    ByteColor *frame_buffer_ptr;
};

#endif // _RENDERINFO_H
