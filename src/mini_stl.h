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

template <class T, int max_size>
class BoundedArray {
public:
    BoundedArray():_currentSize(0) {}
    T& operator[](int idx) {
        DASSERT(idx < _currentSize, "BoundedArray operator[]");
        return _elements[idx];
    }
    void push_back(const T& t) {
        DASSERT(_currentSize < max_size, "BoundedArray::push_back");
        _elements[_currentSize++] = t;
    }
    void pop_back() {
        DASSERT(_currentSize > 0, "BoundedArray::pop_back");
         _currentSize--; 
    }
    bool empty() { return _currentSize == 0; }
    operator T*() { return _elements; }

    T _elements[max_size];
    int _currentSize;
};

class Pool {
    static char pool_data[POOL_SIZE];
public:
    static size_t pool_offset;
    static void *inner_alloc(size_t size) {
        DASSERT(pool_offset < sizeof(pool_data) - size, "Out of heap...");
        void *ptr = reinterpret_cast<void*>(&pool_data[pool_offset]);
        pool_offset += size;
        return ptr;
    };

    template <class T>
    static T* alloc() {
        T* p = reinterpret_cast<T*>(Pool::inner_alloc(sizeof(T)));
        return p;
    }
    static void pool_stats() {
        Serial.print(F("Free pool left:    "));
        Serial.println(sizeof(Pool::pool_data) - Pool::pool_offset);
    }
};

template <class T>
class list {
    struct boxData {
        T _data;
        struct boxData *_next;
    };
    typedef struct boxData box;

    box _head;

    struct iteratorData {
        box *_p;
        iteratorData(box *p)
            :_p(p) {}
        iteratorData(box& rhs)
            :_p(&rhs) {}
        void operator++() {
            DASSERT(_p, "Invalid list iteration...");
            _p = _p->_next;
        }
        T& operator* () {
            return _p->_next->_data;
        }
        T* operator-> () {
            return &_p->_next->_data;
        }
        bool operator !=(const struct iteratorData& rhs) {
            return NULL != _p->_next;
        }
    };
public:
    typedef struct iteratorData iterator;
    list() {
        _head._next = NULL;
    }
    void push_back(const T& t) {
        auto *ptr = Pool::alloc<box>();
        ptr->_next = _head._next;
        ptr->_data = t;
        _head._next = ptr;
        //dprintf("push_back() returns: 0x%x\n", _head._next);
    }
    iterator begin() {
        //dprintf("begin() returns: 0x%x\n", _head._next);
        return iterator(_head);
    }
    iterator end() {
        return iterator(NULL);
    }
};

#endif
