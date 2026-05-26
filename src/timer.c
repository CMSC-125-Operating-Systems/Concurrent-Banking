#include "timer.h"

void timer_start(Timer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

void timer_stop(Timer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
}

double timer_elapsed_ms(const Timer *timer) {
    double start_ms;
    double end_ms;

    start_ms = (double)timer->start.tv_sec * 1000.0 + (double)timer->start.tv_nsec / 1000000.0;
    end_ms = (double)timer->end.tv_sec * 1000.0 + (double)timer->end.tv_nsec / 1000000.0;
    return end_ms - start_ms;
}
