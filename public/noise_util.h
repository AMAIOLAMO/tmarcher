#pragma once
#ifndef _NOISE_UTIL_H
#define _NOISE_UTIL_H

#include "cxvec.h"

// returns a uniformed random unit vector given a grid position
Vec2f rand_gradient2i(int ix, int iy);

// calculates a random gradient given a 2d coordinates
float perlin2f(float x, float y);

// returns the fractal brownian motion(layered perlin noise) given the amount of octaves
float fBM_perlin2f(float x, float y, int octaves, float self_similarity = .5f);


#endif // _NOISE_UTIL_H
