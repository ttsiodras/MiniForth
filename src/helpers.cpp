#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "helpers.h"

#ifdef __x86_64

void flash_printf(const __FlashStringHelper *fmt, ...)
{
    static char msg[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    Serial.print(msg);
    Serial.flush();
}

#else

void flash_printf(const __FlashStringHelper *fmt, ...)
{
    static char msg[128];
    // Steal space from the pool (temporarily)
    int fmtLen = strlen_P((PGM_P)fmt);
    char *p = (char *) malloc(fmtLen);
    strcpy_P(p, (PGM_P)fmt);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), p, ap);
    va_end(ap);
    free(p);
    Serial.print(msg);
    Serial.flush();
}

#endif

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

static int freeArduinoHeap() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#elif defined(__x86_64)
  return 1000000;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

void memory_info()
{
#ifndef __x86_64
    //dprintf("Stack size:        %d\n", RAMEND - SP);
    dprintf("Free Arduino heap: %d\n", freeArduinoHeap());
#endif
}
