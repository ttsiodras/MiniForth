#ifndef __STACK_NODE_H__
#define __STACK_NODE_H__

#include "miniforth.h"

// These are the nodes we store in our run-time Forth stack.
// Made to use as little of our precious-SRAM as possible...
class StackNode {
public:
    // We are either a literal - in which case we are just
    // an intVal word - or we are... something else, that
    // the dictionary will tell us how to handle.
    enum StackCompiledNodeType { UNKNOWN, LIT, PTR } _kind;
    union StackData {
        int intVal;
        DictionaryPtr dictPtr;
    } _u;

    StackNode():_kind(UNKNOWN) { _u.intVal = -1; }
    static StackNode makeNr(int intVal);
    static StackNode makePtr(DictionaryPtr dictPtr);
    void dots();
};

#endif
