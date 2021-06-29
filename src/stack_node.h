#ifndef __STACK_NODE_H__
#define __STACK_NODE_H__

#include "miniforth.h"

class StackNode {
public:
    enum StackCompiledNodeType { UNKNOWN, LIT, IDX } _kind;
    union StackData {
        int intVal;
        DictionaryPtr dictPtr;
    } _u;

    StackNode():_kind(UNKNOWN) { _u.intVal = -1; }
    static StackNode makeNr(int intVal);
    static StackNode makePtr(DictionaryPtr dictPtr);
    bool isLiteral() { return _kind == LIT; }
    int getLiteralValue();
    void dots();
};

#endif
