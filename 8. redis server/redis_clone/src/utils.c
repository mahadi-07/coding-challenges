#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

uint64_t now_ms(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC); 
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

bool is_number(const char *s)
{
    char *end;
    errno = 0;

    long value = strtol(s, &end, 10);

    if(s == end)
        return false;
    
    if(*end != '\0')
        return false;
    
    if(errno == ERANGE)
        return false;
    
    if(value < INT_MIN || value > INT_MAX) 
        return false;

    return true;
}