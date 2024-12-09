#pragma once
#ifndef _TERM_UTIL_H
#define _TERM_UTIL_H

#include <string>

bool try_input_int(int *result_ptr, bool (*predicate)(int) = nullptr);

void wait(int ms);

struct Color {
    char r, g, b;
};

enum class FgColor {
    Black = 0,
    Red = 31, Green = 32, Yellow = 33, Blue = 34,
    Magenta = 35, Cyan = 36, White = 37
};

typedef unsigned char byte;

struct ByteColor {
public:
    ByteColor(byte r, byte g, byte b);
    ByteColor(byte brightness);
    ByteColor(float brightness);
    ByteColor();

    ByteColor operator+(const ByteColor other);
    ByteColor operator-(const ByteColor other);
    ByteColor operator*(const float mask);

    byte r, g, b;

    static ByteColor Black();
    static ByteColor White();
    static ByteColor Red();
    static ByteColor Green();
    static ByteColor Blue();
};

void set_fg_color(FgColor color);
void set_fg_color(byte r, byte g, byte b);
void set_fg_color(ByteColor color);

size_t fread_byte_color(FILE *file_ptr, ByteColor *color_ptr);
size_t fread_byte_color(FILE *file_ptr, byte *color_ptr);

// linear interpolating color
ByteColor lerp_color(ByteColor a, ByteColor b, float t);

void reset_color();

void clear_screen();

void move_cursor(int row, int col);

#endif // _TERM_UTIL_H
