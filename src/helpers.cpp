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
    size_t oldPoolLevel = Pool::pool_offset;
    int fmtLen = strlen_P((PGM_P)fmt);
    char *p = reinterpret_cast<char *>(Pool::inner_alloc(fmtLen+1));
    strcpy_P(p, (PGM_P)fmt);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), p, ap);
    va_end(ap);
    // restore Pool!
    Pool::pool_offset = oldPoolLevel;
    Serial.print(msg);
    Serial.flush();
}

#endif

void memory_info(unsigned freeListTotals)
{
#ifndef __x86_64
    dprintf("Stack used so far: %d bytes\n", RAMEND - SP);
#endif
    Pool::pool_stats(freeListTotals);
}
