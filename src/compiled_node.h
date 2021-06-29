#ifndef __COMPILED_NODE_H__
#define __COMPILED_NODE_H__

#include "errors.h"

struct CompiledNode {

    // The kinds of Forth constructs we support
    enum CompiledNodeType {
        LITERAL,
        CONSTANT,
        VARIABLE,
        C_FUNC,
        WORD
    };

    // Type used for C_FUNC callbacks
    typedef SuccessOrFailure (*FuncPtr)();

    CompiledNodeType _kind;
    union UnionData {
        struct {
            DictionaryPtr _dictPtr; // unused, but needed for alignment
            int _intVal;
        } _literal;
        struct {
            DictionaryPtr _dictPtr;
            int _intVal;
        } _constant;
        struct {
            DictionaryPtr _dictPtr;
            int *_memoryPtr;
            // later, for allot
            //int _memorySize;
        } _variable;
        struct {
            DictionaryPtr _dictPtr;
            FuncPtr _funcPtr;
        } _function;
        struct {
            DictionaryPtr _dictPtr;
        } _word;
    } _u;

    // The memory used to store things like strings/arrays
    static int      _memory[MEMORY_SIZE];
    static unsigned _currentMemoryOffset;

    CompiledNode();
    static CompiledNode makeLiteral(int intVal);
    static CompiledNode makeConstant(DictionaryPtr dictPtr);
    static CompiledNode makeVariable(DictionaryPtr dictPtr, int intVal);
    static CompiledNode makeCFunction(DictionaryPtr dictPtr, FuncPtr funcPtr);
    static CompiledNode makeWord(DictionaryPtr dictPtr);

    void id();
    void dots();
    SuccessOrFailure execute();
    int getLiteralValue();
    void setConstantValue(int intVal);
    void setVariableValue(int intVal);
    int getVariableValue();

    // later, for allot
    // void setVariableSize(int newSize)
    // {
    //     dassert(_kind == VARIABLE);
    //     _currentMemoryOffset -= sizeof(int);
    //     _memoryOffset = _currentMemoryOffset;
    //     _memorySize = newSize;
    //     _currentMemoryOffset += _memorySize;
    // }
};

#endif
