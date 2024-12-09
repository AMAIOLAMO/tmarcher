#include "../public/math_util.h"

float stepf(float threshold, float value) {
    return value < threshold ? 0 : 1;
}

float lerpf(float a, float b, float t) {
    return a * (1.0 - t) + b * t;
}

float minf(float a, float b) {
    return a < b ? a : b;
}

float maxf(float a, float b) {
    return a > b ? a : b;
}

float clampf(float v, float min, float max) {
    if(v < min)
        return min;

    if(v > max)
        return max;

    return v;
}


float saturatef(float v) {
    return clampf(v, 0, 1);
}


void rotate_raw2(float *x_ptr, float *y_ptr, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    
    
    float new_x = c * (*x_ptr) - s * (*y_ptr);
    *y_ptr = s * (*x_ptr) + c * (*y_ptr);
    *x_ptr = new_x;
}

// returns the fractional part of x
float fract(float x) {
    return x - floor(x);
}

float modulof(float x, float y) {
    return x - y * floor(x / y);
}
