#include "../public/time_util.h"

steady_clock::time_point time_mark_now() {
    return steady_clock::now();
}

double get_delta(time_mark_t prev_time_mark) {
    return duration_cast<duration<double>>(time_mark_now() - prev_time_mark).count();
}

