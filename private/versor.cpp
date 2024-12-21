#include "../public/versor.h"
#include <stdio.h>

// represents some kind of rotation in 3D space
Versor::Versor(float w, float i, float j, float k) : w(w), i(i), j(j), k(k) {
}

Versor Versor::conjugate() const {
    // e^theta * u = cos(theta)  + u sin(theta)
    // e^-theta * u = cos(theta) - u sin(theta)
    return {
        w, -i, -j, -k
    };
}

float Versor::angle() const {
    return acosf(w);
}

Versor versor_identity() {
    return { 1, 0, 0, 0 };
}

float versor_len(Versor v) {
    return sqrtf(v.w * v.w + v.i * v.i + v.j * v.j + v.k * v.k);
}

Versor versor_norm(Versor v) {
    float scale_norm = 1.0f / versor_len(v);

    v.w *= scale_norm;
    v.i *= scale_norm;
    v.j *= scale_norm;
    v.k *= scale_norm;

    return v;
}

Versor versor_from_axis_rot(Vec3f axis, float rad) {
    float s = sinf(rad * .5f);
    axis = Vec3f_norm(axis);

    return versor_norm({
        cosf(rad * .5f), s * axis.x, s * axis.y, s * axis.z
    });
}


Versor versor_mul(Versor a, Versor b) {
    return versor_norm({
        fmaf(a.w, b.w, fmaf(-a.i, b.i, fmaf(-a.j, b.j, -a.k * b.k))),
        fmaf(a.w, b.i, fmaf(a.i,  b.w, fmaf(a.j,  b.k, -a.k * b.j))),
        a.w * b.j - a.i * b.k + a.j * b.w + a.k * b.i,
        a.w * b.k + a.i * b.j - a.j * b.i + a.k * b.w
    });
}

Vec3f versor_rot3f(Versor q, Vec3f v) {
    const float ii = q.i * q.i;
    const float jj = q.j * q.j;
    const float kk = q.k * q.k;

    // manual matrix multiplication unrolled
    return {
        v.x * (1 - 2 * jj - 2 * kk) + v.y * (2 * q.i * q.j - 2 * q.w * q.k) + v.z * (2 * q.i * q.k + 2 * q.w * q.j),
        v.x * (2 * q.i * q.j + 2 * q.w * q.k) + v.y * (1 - 2 * ii - 2 * kk) + v.z * (2 * q.j * q.k - 2 * q.w * q.i),
        v.x * (2 * q.j * q.k - 2 * q.w * q.j) + v.y * (2 * q.j * q.k + 2 * q.w * q.i) + v.z * (1 - 2 * ii - 2 * jj)
    };
}

void versor_fprint(FILE *file, Versor v) {
    fprintf(file, "<%.2f, %.2f, %.2f, .%2f>", v.w, v.i, v.j, v.k);
}
