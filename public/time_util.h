#pragma once
#ifndef _TIME_UTIL_H
#define _TIME_UTIL_H

#include <chrono>

using namespace std::chrono;

typedef steady_clock::time_point time_mark_t;

steady_clock::time_point time_mark_now();

#ifndef S2MS
    #define S2MS 1000.0
#endif

// FIX: pretty ambiguous name for a function
double get_delta(time_mark_t prev_time_mark);

#endif // _TIME_UTIL_H
