#include "errors.h"
#include "helpers.h"

#ifndef __NATIVE_BUILD__

SuccessOrFailure error(const __FlashStringHelper *msg) {
    Serial.print(F("[x] "));
    Serial.println(msg);
    return FAILURE;
}

SuccessOrFailure error(const __FlashStringHelper *msg, const char *data) {
    Serial.print(F("[x] "));
    Serial.print(msg);
    Serial.println(data);
    return FAILURE;
}

SuccessOrFailure error(const __FlashStringHelper *msg, const __FlashStringHelper *data) {
    Serial.print(F("[x] "));
    Serial.print(msg);
    Serial.println(data);
    return FAILURE;
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

