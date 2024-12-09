#pragma once
#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "cxvec.h"

struct ByteColor;

struct Texture {
    ByteColor *data;

    int width;
    int height;
};

enum TextureSampleMode {
    TSM_REPEAT,
    TSM_CLAMP
};

// loads a ppm image as a readable texture
Texture texture_load_file(const char *file_path);

ByteColor texture_get_pixel(Texture t, int x, int y);

ByteColor texture_get_pixel(Texture t, Vec2i pos);

ByteColor texture_sample(Texture t, Vec2f uv, TextureSampleMode sample_mode = TSM_CLAMP);

void unload_texture(Texture t);


#endif // _TEXTURE_H
