#pragma once
#ifndef _ROTORS_H
#define _ROTORS_H

#include "cxvec.h"


// represents some kind of rotation in 3D space
struct Versor {
    float w, i, j, k;

    Versor(float w, float i, float j, float k);

    Versor conjugate() const;

    float angle() const;
};

// TODO: put all of this in the versor struct, instead of putting them outside
Versor versor_identity();

float versor_len(Versor v);

Versor versor_norm(Versor v);

Versor versor_from_axis_rot(Vec3f axis, float rad);

Versor versor_mul(Versor a, Versor b);

Vec3f versor_rot3f(Versor q, Vec3f v);

void versor_fprint(FILE *file, Versor v);

#endif // _ROTORS_H
