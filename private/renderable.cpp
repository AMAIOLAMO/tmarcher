#include "../public/renderable.h"
#include "../public/term_util.h"
#include "../public/material.h"

Renderable::Renderable(SDFable *sdfable_ptr, ColorFragmentable *fragmentable_ptr) :
    _sdfable_ptr(sdfable_ptr), _fragmentable_ptr(fragmentable_ptr) {
    assert(_sdfable_ptr != nullptr);
    assert(_fragmentable_ptr != nullptr);
}

float Renderable::dist(Vec3f pt) const {
    assert(_sdfable_ptr != nullptr);

    return _sdfable_ptr->dist(pt);
}


ByteColor Renderable::frag(Vec3f world_pt, float time) const {
    assert(_fragmentable_ptr != nullptr);

    return _fragmentable_ptr->frag(_sdfable_ptr, world_pt, time);
}

void Renderable::set_fragmentable_ptr(ColorFragmentable *fragmentable_ptr) {
    assert(_fragmentable_ptr != nullptr);

    _fragmentable_ptr = fragmentable_ptr;
}
