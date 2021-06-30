#ifndef __MINI_STL_H__
#define __MINI_STL_H__

#include <string.h>

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
tuple<T1, T2> make_tuple(const T1& t1, const T2& t2) 
{
    return tuple<T1, T2>(t1, t2);
}

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
    static void pool_stats(int freeListTotals) {
        Serial.print(F("Free pool left:    "));
        Serial.print(sizeof(Pool::pool_data) - Pool::pool_offset + freeListTotals);
        Serial.println(F(" bytes"));
    }
};

class string {
    char *_p;
public:
    string():_p(NULL) {}
    string(const char *p) {
        size_t len = strlen(p);
        _p = reinterpret_cast<char *>(Pool::inner_alloc(len+1));
        strcpy(_p, p);
    }
    const char *c_str() { return _p; }
    bool empty() { return _p == NULL; }
    void clear() { _p = NULL; }
#ifndef __x86_64
    string(const __FlashStringHelper *p) {
        size_t len = strlen_P((PGM_P)p);
        _p = reinterpret_cast<char *>(Pool::inner_alloc(len+1));
        strcpy_P(_p, (PGM_P)p);
    }
#endif
    operator char*() { return _p; }
    char& operator[](int idx) {
        DASSERT(_p != NULL,
                "string is empty, yet operator[] called...");
        return _p[idx];
    }
};

template <class T>
class forward_list {
public:
    struct boxData {
        T _data;
        struct boxData *_next;
    };
    typedef struct boxData box;
    static unsigned _freeListMemory;
private:
    box *_head;
    static box *_freeList;

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
            return _p->_data;
        }
        T* operator-> () {
            return &_p->_data;
        }
        bool operator !=(const struct iteratorData& rhs) {
            // This operator is only used in for-loops with auto:
            //
            //    for(auto v&: listVar) ...
            //
            // ...so we just need to terminate the iteration
            // when there are no more data.
            (void) rhs;
            return NULL != _p;
        }
    };
public:
    typedef struct iteratorData iterator;
    forward_list() {
        _head = NULL;
    }
    bool empty() {
        return _head == NULL;
    }
    void push_back(const T& t) {
        box *ptr;
        if (!_freeList)
            ptr = Pool::alloc<box>();
        else {
            ptr = _freeList;
            _freeList = _freeList->_next;
            _freeListMemory -= sizeof(box);
        }
        ptr->_next = _head;
        ptr->_data = t;
        _head = ptr;
    }
    void pop_front() {
        DASSERT(_head, "pop_front called with empty list...");
        box *newHead = _head->_next;
        _head->_next = _freeList;
        _freeList = _head;
        _head = newHead;
        _freeListMemory += sizeof(box);
    }
    iterator begin() {
        return iterator(_head);
    }
    iterator end() {
        return iterator(NULL);
    }
};

#endif
