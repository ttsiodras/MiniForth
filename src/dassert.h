#ifndef __DASSERT_H__
#define __DASSERT_H__

#include <Arduino.h>

#define DASSERT(condition, msg)                  \
do {                                             \
    if (!(condition)) {                          \
        Serial.print(F("Assertion: \""));        \
        Serial.print(F(#condition));             \
        Serial.print(F("\" failed at "));        \
        Serial.print(F(__FILE__));               \
        Serial.print(F(":"));                    \
        Serial.print(__LINE__);                  \
        Serial.println(F("\n" msg));             \
        Serial.println(F("Halting execution.")); \
        Serial.flush();                          \
        exit(0);                                 \
    }                                            \
} while(0);

#endif
