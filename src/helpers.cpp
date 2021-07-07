#include <Arduino.h>

#include <stdio.h>
#include <stdarg.h>

#include "helpers.h"

#ifdef __x86_64__

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
    // Steal space from the pool (temporarily)
    //
    // We need space for two reasons: one is to create a normal
    // string out of our Flash-based format, since vsnprintf_P
    // is apparently not there:
    size_t oldPoolLevel = Pool::pool_offset;
    int fmtLen = strlen_P((PGM_P)fmt);
    char *p = reinterpret_cast<char *>(Pool::inner_alloc(fmtLen+1));
    strcpy_P(p, (PGM_P)fmt);

    // The second reason is our target itself!
    va_list ap;
    va_start(ap, fmt);
    fmtLen = vsnprintf(NULL, 0, p, ap) + 1;
    va_end(ap);

    char *msg = reinterpret_cast<char *>(Pool::inner_alloc(fmtLen+1));
    va_start(ap, fmt);
    vsnprintf(msg, fmtLen+1, p, ap);
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
    dprintf("\nStack used so far: %d/%d bytes\n",
            RAMEND - SP, STACK_SIZE);
#endif
    Pool::pool_stats(freeListTotals);
}
