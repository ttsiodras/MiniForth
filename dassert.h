#ifndef __DASSERT_H__
#define __DASSERT_H__

#define DASSERT(condition)                                         \
do {                                                               \
    extern void dprintf(const char *fmt, ...);                     \
    if (!(condition))                                              \
        dprintf("Assertion failed: %s:%d \n", __FILE__, __LINE__); \
} while(0);

#endif
