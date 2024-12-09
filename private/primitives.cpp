#include "../public/primitives.h"
#include "../public/math_util.h"
#include "../public/noise_utils.h"

Plane::Plane(float y) : y(y) {}

float Plane::dist(Vec3f pt) const {
    return pt.y - y;
}


Terrain::Terrain(float frequency, float amplitude, int octaves) :
    _frequency(frequency), _amplitude(amplitude), _octaves(octaves) {
    assert(_frequency != 0);
}

float Terrain::dist(Vec3f pt) const {
    return pt.y - fBM_perlin2f(pt.x * _frequency, pt.z * _frequency, _octaves) * _amplitude;
}


Sphere::Sphere(Vec3f pos, float radius) : pos(pos), radius(radius) {}

float Sphere::dist(Vec3f pt) const {
    return Vec3f_lengthf(pt - pos) - radius;
}


Cube::Cube(Vec3f pos, Vec3f size) : pos(pos), size(size) {}

float Cube::dist(Vec3f pt) const {
    return Vec3f_lengthf(Vec3f{
        maxf(fabsf(pt.x - pos.x) - size.x, 0.),
        maxf(fabsf(pt.y - pos.y) - size.y, 0.),
        maxf(fabsf(pt.z - pos.z) - size.z, 0.)
    });
}


Capsule::Capsule(Vec3f a, Vec3f b, float radius) : a(a), b(b), radius(radius) {
}

float Capsule::dist(Vec3f pt) const {
    Vec3f ab = b - a;
    Vec3f ap = pt - a;

    float t = Vec3f_dot(ab, ap) / Vec3f_dot(ab, ab);
    t = saturatef(t);

    Vec3f c = a + ab * t;

    return Vec3f_lengthf(pt - c) - radius;
}


Tetrahedron::Tetrahedron(int iterations, float scale) : _scale(scale), _iterations(iterations) {
    assert(_iterations >= 1);
}

float Tetrahedron::dist(Vec3f pt) const {
    pt.x = modulof(pt.x, 16.0) - 8.0f;
    pt.y = modulof(pt.y, 16.0) - 8.0f;
    pt.z = modulof(pt.z, 16.0) - 8.0f;

    return Vec3f_lengthf(pt) - 3.f;
}
