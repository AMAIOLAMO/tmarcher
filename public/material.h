#pragma once
#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "cxvec.h"
#include "sdfable.h"
#include "term_util.h"

// represents some colors
class ColorFragmentable {
public:
    virtual ByteColor frag(SDFable *sdfable_ptr, Vec3f world_pt, float time) const = 0;
};

class SolidMaterial : public ColorFragmentable {
public:
    SolidMaterial(ByteColor albedo);

    ByteColor albedo;

    virtual ByteColor frag(SDFable *sdfable_ptr, Vec3f world_pt, float time) const;
};

typedef ByteColor(*frag_func_t)(SDFable *sdfable_ptr, Vec3f world_pt, float time);

class FuncMaterial : public ColorFragmentable {
public:
    FuncMaterial(frag_func_t frag_func);

    ByteColor frag(SDFable *sdfable_ptr, Vec3f world_pt, float time) const override;

private:
    frag_func_t _frag_func;
};


#endif // _MATERIAL_H
