#include "../public/texture.h"
#include "../public/term_util.h"
#include "../public/math_util.h"
#include <assert.h>

static void fread_token(FILE *file_ptr, char *buffer, size_t max_char) {
    size_t idx = -1;
    do {
        idx += 1;

        if(idx + 1 >= max_char) {
            buffer[max_char - 1] = '\0';
            return;
        }

        fread(&buffer[idx], sizeof(byte), 1, file_ptr);
    } while(isalnum(buffer[idx]));
    // else end

    buffer[idx] = '\0'; // null terminate
}


Texture texture_load_file(const char *file_path) {
    FILE *f = fopen(file_path, "r");

    Texture t;

    char buffer[256];
    fread_token(f, buffer, 256); // PPM VERSION
    fread_token(f, buffer, 256); // WIDTH
    t.width = atoi(buffer);

    fread_token(f, buffer, 256); // HEIGHT 
    t.height = atoi(buffer);

    fread_token(f, buffer, 256); // COLOR WIDTH

    t.data = (ByteColor*)malloc(sizeof(ByteColor) * t.width * t.height);

    for(int i = 0; i < t.height * t.width; i++)
        fread_byte_color(f, &t.data[i]);

    fclose(f);

    return t;
}


ByteColor texture_get_pixel(Texture t, int x, int y) {
    assert(x >= 0 && y >= 0 && x < t.width && y < t.height);

    return t.data[y * t.width + x];
}

inline ByteColor texture_get_pixel(Texture t, Vec2i pos) {
    return texture_get_pixel(t, pos.x, pos.y);
}


ByteColor texture_sample(Texture t, Vec2f uv, TextureSampleMode sample_mode) {
    // default is clamp
    if(sample_mode == TSM_CLAMP) {
        uv.x = saturatef(uv.x);
        uv.y = saturatef(uv.y);
    }

    if(sample_mode == TSM_REPEAT) {
        uv.x = fract(uv.x);
        uv.y = fract(uv.y);
    }

    int px = maxf(t.width - 1, 0) * uv.x;
    int py = maxf(t.height - 1, 0) * uv.y;

    return texture_get_pixel(t, (Vec2i){px, py});
}

void unload_texture(Texture t) {
    free(t.data);
}
