#include "../public/term_util.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cstdarg>
#include <assert.h>

#ifdef _WIN32
#include <conio.h>
#endif

#include "../public/math_util.h"


bool try_input_int(int *result_ptr, bool (*predicate)(int)) {
    if(std::cin >> *result_ptr)
        return predicate ? predicate(*result_ptr) : *result_ptr;
    // else

    return false;
}


void wait(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void set_fg_color(FgColor color) {
    printf("\033[%dm", (int)color);
}

void set_fg_color(byte r, byte g, byte b) {
    printf("\033[38;2;%d;%d;%dm", r, g, b);
}

void set_fg_color(ByteColor color) {
    set_fg_color(color.r, color.g, color.b);
}

void reset_color() {
    fwrite("\033[0m", sizeof("\033[0m"), 1, stdout);
}

size_t fread_byte_color(FILE *file_ptr, ByteColor *color_ptr) {
    return fread(color_ptr, sizeof(byte), 3, file_ptr);
}

size_t fread_byte_color(FILE *file_ptr, byte *color_ptr) {
    return fread(color_ptr, sizeof(byte), 3, file_ptr);
}


void clear_screen() {
#ifdef _WIN32
    clrscr();
#endif

#ifdef linux
    puts("\033[2J\033[1;1H");
#endif
}

void move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

ByteColor::ByteColor(byte r, byte g, byte b) : r(r), g(g), b(b) { }
ByteColor::ByteColor() : ByteColor(0, 0, 0) { }
ByteColor::ByteColor(byte brightness) : ByteColor(brightness, brightness, brightness) {}
ByteColor::ByteColor(float brightness) : ByteColor((byte)(brightness * 255)) {}

ByteColor ByteColor::operator+(const ByteColor other) {
    return {
        (byte)std::min(r + other.r, 255),
        (byte)std::min(g + other.g, 255),
        (byte)std::min(b + other.b, 255)
    };
}

ByteColor ByteColor::operator-(const ByteColor other) {
    return {
        (byte)std::max(r - other.r, 0),
        (byte)std::max(g - other.g, 0),
        (byte)std::max(b - other.b, 0)
    };
}

ByteColor ByteColor::operator*(const float mask) {
    return {
        (byte)std::min((int)(r * mask), 255),
        (byte)std::min((int)(g * mask), 255),
        (byte)std::min((int)(b * mask), 255)
    };
}

ByteColor ByteColor::Black() {
    return {0, 0, 0};
}
ByteColor ByteColor::White() {
    return {255, 255, 255};
}
ByteColor ByteColor::Red() {
    return {255, 0, 0};
}
ByteColor ByteColor::Green() {
    return {0, 255, 0};
}
ByteColor ByteColor::Blue() {
    return {0, 0, 255};
}

// same as above, but for linear interpolating a color for each channel
ByteColor lerp_color(ByteColor a, ByteColor b, float t) {
    return {
        (byte)lerpf(a.r, b.r, t),
        (byte)lerpf(a.g, b.g, t),
        (byte)lerpf(a.b, b.b, t)
    };
}

