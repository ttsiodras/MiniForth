#ifndef __ERRORS_H__
#define __ERRORS_H__

#include "mini_stl.h"

typedef enum SuccessOrFailure {
    FAILURE,
    SUCCESS
} SuccessOrFailure;

template <class T>
class Optional : public tuple<SuccessOrFailure, T>
{
public:
    Optional(SuccessOrFailure retCode):
        tuple<SuccessOrFailure, T>(retCode, T())
    {}
    Optional(const T& t):tuple<SuccessOrFailure, T>(SUCCESS, t)
    {}
};

typedef Optional<int> EvalResult;

SuccessOrFailure error(const __FlashStringHelper *msg);
SuccessOrFailure error(const __FlashStringHelper *msg, const char *data);
#ifndef __x86_64__
SuccessOrFailure error(const __FlashStringHelper *msg, const __FlashStringHelper *data);
#endif

#endif
