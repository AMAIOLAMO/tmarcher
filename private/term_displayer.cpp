#include "../public/term_displayer.h"

void TerminalDisplayer::display(ByteColor *frame_buffer, size_t width, size_t height) const {
    move_cursor(0, 0);
    for(size_t y = 0; y < height; y++) {
        for(size_t x = 0; x < width; x++) {
            ByteColor color = frame_buffer[y * width + x];

            set_fg_color(color);
            fwrite("██", sizeof("██"), 1, stdout);
        }
        fwrite("\n", sizeof(char), 1, stdout);
    }
}
