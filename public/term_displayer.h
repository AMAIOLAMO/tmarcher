#pragma once
#include "term_util.h"
#ifndef _TERM_DISPLAYER_H
#define _TERM_DISPLAYER_H

// NOTE: For now there isn't any reason to have instances of this class due to it being
// just a single function
class TerminalDisplayer {
public:
    void display(ByteColor *frame_buffer, size_t width, size_t height) const;
};

#endif // _TERM_DISPLAYER_H
