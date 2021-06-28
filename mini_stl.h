#ifndef __MINI_STL_H__
#define __MINI_STL_H__

#include "dassert.h"

template <class T1, class T2>
class tuple {
public:
    T1 _t1;
    T2 _t2;
    tuple(const T1& t1, const T2& t2)
        :_t1(t1), _t2(t2) {}
    tuple()
        :_t1(T1()), _t2(T2()) {}
    tuple(const tuple& rhs) {
        _t1 = rhs._t1;
        _t2 = rhs._t2;
    }
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
        if(idx >= max_size) {
            dprintf("Indexing %d >= %d", idx, max_size);
            DASSERT(false);
        }
        return _elements[idx];
    }
};

template <class T, int max_size>
class BoundedArray {
public:
    BoundedArray():_currentSize(0) {}
    T& operator[](int idx) {
        if(idx >= _currentSize) {
            dprintf("Indexing %d >= %d", idx, _currentSize);
            DASSERT(false);
        }
        return _elements[idx];
    }
    void push_back(const T& t) {
        DASSERT(_currentSize < max_size);
        _elements[_currentSize++] = t;
    }
    void pop_back() {
        DASSERT(_currentSize > 0);
         _currentSize--; 
    }
    bool empty() { return _currentSize == 0; }
    operator T*() { return _elements; }

    T _elements[max_size];
    int _currentSize;
};

#endif
