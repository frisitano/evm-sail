/* Freestanding <time.h> shim. The guest has no real clock. */
#ifndef ZKVM_FREESTANDING_TIME_H
#define ZKVM_FREESTANDING_TIME_H
typedef long time_t;
struct timespec { time_t tv_sec; long tv_nsec; };
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
time_t time(time_t *tloc);
int clock_gettime(int clk_id, struct timespec *tp);
#endif
