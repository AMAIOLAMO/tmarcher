#include "../public/noise_util.h"

static inline float in_out_cubic(float a, float b, float t) {
    return (b - a) * (3.0 - t * 2.0) * t * t + a;
}

Vec2f rand_gradient2i(int ix, int iy) {
    // no precomputed gradients means this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;

    unsigned a = ix, b = iy;
    a *= 3284157443;

    b ^= a << s | a >> (w - s);
    b *= 1911520717;

    a ^= b << s | b >> (w - s);
    a *= 2048419325;

    float random = a * (3.14159265 / ~(~0u >> 1)); // within [0, 2*pi]
    
    return {
        cosf(random), sinf(random)
    };
}

static float dot_grid_rand_gradient2f(int ix, int iy, float x, float y) {
    Vec2f gradient = rand_gradient2i(ix, iy);
    
    // distance vector
    Vec2f dist_vec{
        x - (float)ix,
        y - (float)iy
    };

    return Vec2f_dot(gradient, dist_vec);
}


// calculates a random gradient given a 2d coordinates
float perlin2f(float x, float y) {
    // corner of the cell coordinates
    int ix = floor(x);
    int iy = floor(y);
    
    // weights for the dot within the grid
    float tx = x - (float)ix;
    float ty = y - (float)iy;

    // interpolate top two dot products
    float tl = dot_grid_rand_gradient2f(ix,     iy,     x, y);
    float tr = dot_grid_rand_gradient2f(ix + 1, iy,     x, y);
    
    float th = in_out_cubic(tl, tr, tx);

    // interpolate bottom two dot products
    float bl = dot_grid_rand_gradient2f(ix,     iy + 1, x, y);
    float br = dot_grid_rand_gradient2f(ix + 1, iy + 1, x, y);

    float bh = in_out_cubic(bl, br, tx);

    // interpolate top to bottom
    return in_out_cubic(th, bh, ty);
}

// returns the fractal brownian motion(layered perlin noise) given the amount of octaves
float fBM_perlin2f(float x, float y, int octaves, float self_similarity) {
    float value = 0;
    float gain = exp2f(-self_similarity);
    float freq = 1.0;
    float amp = 1.0;

    for (int o = 0; o < octaves; ++o) {
        value += perlin2f(x * freq, y * freq) * amp;
        freq *= 2.0f;
        amp *= gain;
    }

    return value;
}
