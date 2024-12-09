#pragma once
#ifndef _CXVEC_H
#define _CXVEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
#define VEC3_CPP_DEF(type, type_name) \
    type_name##_t operator+(const type_name##_t vec) const; \
    type_name##_t operator-(const type_name##_t vec) const; \
    type_name##_t operator*(const type scalar) const; \
    type_name##_t operator-() const; \
    void operator+=(const type_name##_t vec);         \
    void operator-=(const type_name##_t vec);
#else
#define VEC3_CPP_DEF(type, type_name)
#endif

// TODO: Define C++ structures

#define VEC2_DEF(type, type_name) \
    typedef struct type_name##_t { \
        type x, y; \
    } type_name;   \
    type_name type_name##_make(type value);                           \
    type_name type_name##_add(type_name a, type_name b);              \
    type_name type_name##_subtract(type_name a, type_name b);         \
    type_name type_name##_scale(type_name a, type scalar);            \
    float type_name##_lengthf(type_name vec);                         \
    type type_name##_dot(type_name a, type_name b);                   \
    type_name type_name##_norm(type_name vec);

#define VEC3_DEF(type, type_name) \
    typedef struct type_name##_t { \
        type x, y, z; \
        VEC3_CPP_DEF(type, type_name); \
    } type_name;      \
    type_name type_name##_make(type value);                           \
    type_name type_name##_add(type_name a, type_name b);              \
    type_name type_name##_subtract(type_name a, type_name b);         \
    type_name type_name##_subtract_scalar(type_name a, type scalar);  \
    type_name type_name##_scale(type_name a, type scalar);            \
    float type_name##_lengthf(type_name vec);                         \
    type type_name##_dot(type_name a, type_name b);                   \
    type_name type_name##_cross(type_name a, type_name b);            \
    type_name type_name##_norm(type_name vec);                        \
    type_name type_name##_mirror(type_name vec, type_name normal);    \
    type_name type_name##_map(type_name vec, type (*func)(type axis));\
    void type_name##_fprint(FILE *file, type_name vec);

VEC2_DEF(float, Vec2f);
VEC2_DEF(int, Vec2i);

VEC3_DEF(float, Vec3f);
VEC3_DEF(int, Vec3i);

#ifdef __cplusplus
#define VEC3_CPP_IMPL(type, type_name) \
    type_name type_name::operator+(const type_name vec) const { return type_name##_add(*this, vec); } \
    type_name type_name::operator-(const type_name vec) const { return type_name##_subtract(*this, vec); } \
type_name type_name::operator*(const type scalar) const { return type_name##_scale(*this, scalar); } \
    type_name type_name::operator-() const { return type_name##_negate(*this); } \
void type_name::operator+=(const type_name vec) { *this = *this + vec; } \
    void type_name::operator-=(const type_name vec) { *this = *this - vec; }
#else
#define VEC3_CPP_IMPL(type, type_name)
#endif


#ifdef CXVEC_IMPL_ONCE
#define VEC2_IMPL(type, type_name) \
    type_name type_name##_make(type value)                   { return (type_name){value, value}; }               \
    type_name type_name##_add(type_name a, type_name b)      { return (type_name){a.x + b.x, a.y + b.y}; }       \
    type_name type_name##_subtract(type_name a, type_name b) { return (type_name){a.x - b.x, a.y - b.y}; }       \
    type_name type_name##_scale(type_name a, type scalar)    { return (type_name){a.x * scalar, a.y * scalar}; } \
    float type_name##_lengthf(type_name vec)                 { return sqrtf(vec.x * vec.x + vec.y * vec.y); }    \
    type type_name##_dot(type_name a, type_name b)           { return (type)(fma(a.x, b.x, a.y * b.y)); }        \
    type_name type_name##_norm(type_name vec)                { type len = type_name##_lengthf(vec); return len == 0 ? type_name##_make(0) : type_name##_scale(vec, 1.0f / len); } \

#define VEC3_IMPL(type, type_name) \
    type_name type_name##_make(type value)                   { return (type_name){value, value, value}; }                                                                    \
    type_name type_name##_add(type_name a, type_name b)      { return (type_name){a.x + b.x, a.y + b.y, a.z + b.z}; }                                                        \
    type_name type_name##_subtract(type_name a, type_name b) { return (type_name){a.x - b.x, a.y - b.y, a.z - b.z}; }                                                        \
    type_name type_name##_subtract_scalar(type_name a, type scalar) { return (type_name){a.x - scalar, a.y - scalar, a.z - scalar}; }                                        \
    type_name type_name##_scale(type_name a, type scalar)    { return (type_name){a.x * scalar, a.y * scalar, a.z * scalar}; }                                               \
    float type_name##_lengthf(type_name vec)                 { return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z); }                                                \
    type type_name##_dot(type_name a, type_name b)           { return (type)(fma(a.x, b.x, fma(a.y, b.y, a.z * b.z))); }                                                           \
    type_name type_name##_cross(type_name a, type_name b)    { return (type_name){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }                    \
    type_name type_name##_norm(type_name vec)                { type len = type_name##_lengthf(vec); return len == 0 ? type_name##_make(0) : type_name##_scale(vec, 1.0f / len); } \
    type_name type_name##_negate(type_name vec)              { return type_name##_scale(vec, -1); }  \
    type_name type_name##_mirror(type_name vec, type_name normal) { return type_name##_subtract(vec, type_name##_scale(normal, 2 * type_name##_dot(vec, normal))); } \
    type_name type_name##_reflect(type_name incident, type_name normal) { return type_name##_mirror(type_name##_negate(incident), normal); } \
    type_name type_name##_map(type_name vec, type (*func)(type axis)) { assert(func != nullptr); return (type_name){ func(vec.x), func(vec.y), func(vec.z) }; } \
    void type_name##_fprint(FILE *file, type_name vec) { fprintf(file, "<%.2f, %.2f, %.2f>", (float)vec.x, (float)vec.y, (float)vec.z); } \
    VEC3_CPP_IMPL(type, type_name)


VEC2_IMPL(float, Vec2f);
VEC2_IMPL(int, Vec2i);

VEC3_IMPL(float, Vec3f);
VEC3_IMPL(int, Vec3i);

#undef CXVEC_IMPL_ONCE
#endif

#ifdef __cplusplus
}
#endif

#endif // _CXVEC_H
