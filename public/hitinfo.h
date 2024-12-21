#pragma once
#include "cxvec.h"
#ifndef _HITINFO_H
#define _HITINFO_H

struct Renderable;

struct HitInfo {
    Vec3f pt;
    float dist;
    float min_penumbra;
    size_t iter;
    Renderable *renderable_ptr;
    bool hit;
};


#endif // _HITINFO_H
