#ifndef __MINI_STL_H__
#define __MINI_STL_H__

#include "dassert.h"
#include "defines.h"

extern void dprintf(const char *fmt, ...);

template <class T1, class T2>
class tuple {
public:
    T1 _t1;
    T2 _t2;
    tuple(const T1& t1, const T2& t2)
        :_t1(t1), _t2(t2) {}
    tuple() {}
};

template <class T1, class T2>
auto make_tuple(const T1& t1, const T2& t2) -> tuple<T1, T2>
{
    return tuple<T1, T2>(t1, t2);
}

template <int max_size>
class BoundedString {
    char _elements[max_size];
public:
    BoundedString() {
        memset(_elements, 0, sizeof(_elements));
    }
    operator char*() { return _elements; }
    char& operator[](int idx) {
        DASSERT(idx < max_size, "BoundedString operator[]");
        return _elements[idx];
    }
};

#endif
