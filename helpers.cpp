#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>

#include "helpers.h"

void dprintf(const char *fmt, ...)
{
    static char msg[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    Serial.print(msg);
    Serial.flush();
}

SuccessOrFailure error(const char *msg) {
    dprintf("[x] %s\n", msg);
    return FAILURE;
}

SuccessOrFailure error(const char *msg, const char *data) {
    dprintf("[x] %s %s\n", msg, data);
    return FAILURE;
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

static int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

void memory_info()
{
    // dprintf("Stack size: %d\n", RAMEND - SP);
    // dprintf("SP: %d\n", SP);
    // dprintf("RAMEND : %d\n", RAMEND);
    dprintf("Free memory: %d\n", freeMemory());
}

