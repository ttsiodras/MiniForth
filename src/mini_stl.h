#ifndef __MINI_STL_H__
#define __MINI_STL_H__

#include <string.h>

#include "dassert.h"
#include "defines.h"

extern void dprintf(const char *fmt, ...);

// Since ArduinoSTL used way too much space, I made my own mini-STL :-)

// First - the humble tuple. Used for many things, including Optional<T>
// (in which case the first field is a SuccessOrFailure, and the second 
//  field is the actual data).
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

// My own "heap". Calling this a heap is blasphemy, but oh well :-)
class Pool {
    static char pool_data[POOL_SIZE];
public:
    static size_t pool_offset;
    static void clear() {
        memset(pool_data, 0, sizeof(pool_data));
        pool_offset = 0;
    }
    static void *inner_alloc(size_t size) {
        DASSERT(pool_offset < sizeof(pool_data) - size, "Out of heap...");
        void *ptr = reinterpret_cast<void*>(&pool_data[pool_offset]);
        pool_offset += size;
        return ptr;
    };

    // Helper template member - allows us to allocate via e.g. alloc<box>()
    template <class T>
    static T* alloc() {
        T* p = reinterpret_cast<T*>(Pool::inner_alloc(sizeof(T)));
        return p;
    }

    // Statistics.
    static void pool_stats(int freeListTotals) {
        Serial.print(F("Pool  used so far: "));
        Serial.print(Pool::pool_offset - freeListTotals);
        Serial.print(F("/"));
        Serial.print(sizeof(Pool::pool_data));
        Serial.print(F(" bytes"));
    }
};

// Good old strings. I exploit the knowledge that we never release
// strings, once we allocate them... And only implement what I need.
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

    // The Intel monsters don't know what Flash is :-)
    // Protect with #ifndef...
#ifndef __x86_64__
    string(const __FlashStringHelper *p) {
        size_t len = strlen_P((PGM_P)p);
        _p = reinterpret_cast<char *>(Pool::inner_alloc(len+1));
        strcpy_P(_p, (PGM_P)p);
    }
    bool operator==(const __FlashStringHelper *msg) {
        return !strcmp_P(_p, (PGM_P)msg);
    };
#endif
    operator char*() { return _p; }
    bool operator==(const char *msg) {
        return !strcmp(_p, msg);
    };
};

// The main machinery - used everywhere; dictionary, stack, etc
// It is a single-linked list.
template <class T>
class forward_list {
public:
    struct boxData {
        T _data;
        struct boxData *_next;
    };
    typedef struct boxData box;
    // We keep track of released nodes (i.e. pop_front-ed)
    // to re-use them in subsequent allocations via a class-global
    // free-node-list. 
    // Since this is a template, the notion of class-global is a
    // bit more nuanced - see the definition of the relevenant
    // statics at the top of miniforth.cpp.
    static unsigned _freeListMemory;
    static box *_freeList;
private:
    box *_head;

    // Any C++ list needs an iterator! 
    // Which is just a pointer :-)
    struct iteratorData {
        box *_p;
        iteratorData()
            :_p(NULL) {} // Must never be used - here only for
                         // the failure side of Optional<iterator>...
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
            return rhs._p != _p;
        }
        bool operator ==(const struct iteratorData& rhs) {
            return rhs._p == _p;
        }
        box *next() {
            return _p->_next;
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
    void clear() {
        _head = NULL;
    }
    void push_back(const T& t) {
        box *ptr;
        // If we have available nodes in our free list, reuse them!
        if (!_freeList)
            // Otherwise, allocate new one.
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
        // Free the node by putting it on the free list.
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
