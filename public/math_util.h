#pragma once
#ifndef _MATH_UTIL_H
#define _MATH_UTIL_H

#include <math.h>

#ifndef PI
    #define PI 3.1415926535f
#endif

// here are some basic functions, in shaders, like stepf, when a value goes over a threshold it returns 1, otherwise 0
float stepf(float threshold, float value);

// linear interpolation, used for background smoothing, calculates influences linearly from a to b in an additional form
float lerpf(float a, float b, float t);

float minf(float a, float b);

float maxf(float a, float b);

// basic clamping of a value, it returns the value v, but restricted within the range of min and max
float clampf(float v, float min, float max);

// ensures that the value given is constrained within 0 ~ 1 (hence saturated)
float saturatef(float v);

// assumes the given two floating point pointers are used for rotation
void rotate_raw2(float *x_ptr, float *y_ptr, float angle);

// returns the fractional part of x
float fract(float x);

// calculates the floating point modulo between x and y,
// Utilizes: x - y * floor(x / y)
float modulof(float x, float y);


#endif // _MATH_UTIL_H
