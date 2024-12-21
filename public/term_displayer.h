#pragma once
#include "term_util.h"
#ifndef _TERM_DISPLAYER_H
#define _TERM_DISPLAYER_H

class TerminalDisplayer {
public:
    void display(ByteColor *frame_buffer, size_t width, size_t height) const;
};

#endif // _TERM_DISPLAYER_H
