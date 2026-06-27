#include <stdio.h>
#include <time.h>
#include <stdint.h>

uint64_t now_ms(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC); 
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}