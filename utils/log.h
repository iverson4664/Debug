#ifndef __TEST_LOG__
#define __TEST_LOG__

#include <stdio.h>
#include <stdarg.h>

void log(const char* fmt, ...);

#define LOG(fmt, ...) \
    do { \
        log(fmt"\n", ##__VA_ARGS__); \
    } while (0)

#endif // __TEST_LOG__
