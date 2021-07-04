#ifndef __ERRORS_H__
#define __ERRORS_H__

#include "mini_stl.h"

typedef enum SuccessOrFailure {
    FAILURE,
    SUCCESS
} SuccessOrFailure;

template <class T>
class Optional : private tuple<SuccessOrFailure, T>
{
public:
    Optional(SuccessOrFailure retCode):
        tuple<SuccessOrFailure, T>(retCode, T())
    {}
    Optional(const T& t):tuple<SuccessOrFailure, T>(SUCCESS, t)
    {}
    operator bool() {
        return this->_t1 == SUCCESS;
    }
    T& value() { return *&this->_t2; }
};

// Anyone who expects an integer result, but may also fail.
// And no, returning -1 is not an option! A stack node for example,
// can have the LITERAL value -1.
typedef Optional<int> EvalResult;

SuccessOrFailure error(const __FlashStringHelper *msg);
SuccessOrFailure error(const __FlashStringHelper *msg, const char *data);
#ifndef __x86_64__
SuccessOrFailure error(const __FlashStringHelper *msg, const __FlashStringHelper *data);
#endif

#endif
