#pragma once
#ifndef _RENDERABLE_H
#define _RENDERABLE_H

#include "sdfable.h"

struct ColorFragmentable;
struct ByteColor;

class Renderable : public SDFable {
public:
    Renderable(SDFable *sdfable_ptr, ColorFragmentable *fragmentable_ptr);

    virtual float dist(Vec3f pt) const override;

    virtual ByteColor frag(Vec3f world_pt, float time) const;

    void set_fragmentable_ptr(ColorFragmentable *fragmentable_ptr);

protected:
    SDFable *_sdfable_ptr;
    ColorFragmentable *_fragmentable_ptr;
};

#endif // _RENDERABLE_H
