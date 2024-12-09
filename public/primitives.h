#pragma once
#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include "sdfable.h"

class Plane : public SDFable {
public:
    Plane(float y);

    float y;

    virtual float dist(Vec3f pt) const override;
};

class Terrain : public SDFable {
public:
    Terrain(float frequency, float amplitude, int octaves);

    virtual float dist(Vec3f pt) const override;

private:
    float _frequency;
    float _amplitude;
    int _octaves;
};


class Sphere : public SDFable {
public:
    Sphere(Vec3f pos, float radius);
    Vec3f pos;
    float radius;

    virtual float dist(Vec3f pt) const override;
};

class Cube : public SDFable {
public:
    Cube(Vec3f pos, Vec3f size);

    Vec3f pos;
    Vec3f size;

    virtual float dist(Vec3f pt) const override;
};

class Capsule : public SDFable {
public:
    Capsule(Vec3f a, Vec3f b, float radius);

    Vec3f a, b;
    float radius;

    virtual float dist(Vec3f pt) const override;
};


class Tetrahedron : public SDFable {
public:
    Tetrahedron(int iterations, float scale);

    float dist(Vec3f pt) const override;

private:
    float _scale;
    int _iterations;
};

#endif // _PRIMITIVES_H
