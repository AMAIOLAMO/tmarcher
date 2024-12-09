#include "../public/material.h"

SolidMaterial::SolidMaterial(ByteColor albedo) : albedo(albedo) {}

ByteColor SolidMaterial::frag(SDFable *sdfable_ptr, Vec3f world_pt, float time) const {
    (void) sdfable_ptr;
    (void) world_pt;
    (void) time;

    return albedo;
}

typedef ByteColor(*frag_func_t)(SDFable *sdfable_ptr, Vec3f world_pt, float time);

FuncMaterial::FuncMaterial(frag_func_t frag_func) : _frag_func(frag_func) {
    assert(frag_func != nullptr);
}

ByteColor FuncMaterial::frag(SDFable *sdfable_ptr, Vec3f world_pt, float time) const {
    assert(_frag_func != nullptr);

    return _frag_func(sdfable_ptr, world_pt, time);
}

