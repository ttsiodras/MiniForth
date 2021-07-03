#ifndef __COMPILED_NODE_H__
#define __COMPILED_NODE_H__

#include "errors.h"

struct CompiledNode {

    // The kinds of Forth constructs we support
    enum CompiledNodeType {
        UNKNOWN,
        LITERAL,
        STRING,
        CONSTANT,
        VARIABLE,
        C_FUNC,
        WORD
    };

    // Type used for C_FUNC callbacks
    typedef Optional<CompiledNodes::iterator> ExecuteResult;
    typedef ExecuteResult (*FuncPtr)(CompiledNodes::iterator);

    CompiledNodeType _kind;
    union UnionData {
        UnionData() {}
        struct {
            DictionaryPtr _dictPtr; // unused, but needed for alignment
            int _intVal;
        } _literal;
        struct {
            DictionaryPtr _dictPtr; // unused, but needed for alignment
            string _strVal;
        } _string;
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

    string& getWordName() {
        // We deliberately placed the _dictPtr as the first field
        // in all the structs of the _u union.
        return _u._word._dictPtr->_t1;
    }

    // The memory used to store things like strings/arrays
    static int      _memory[MEMORY_SIZE];
    static unsigned _currentMemoryOffset;

    static void memory_clear() {
        memset(_memory, 0, sizeof(_memory));
        _currentMemoryOffset = 0;
    };

    CompiledNode();
    static CompiledNode makeLiteral(int intVal);
    static CompiledNode makeString(const char *p);
    static CompiledNode makeConstant(DictionaryPtr dictPtr);
    static CompiledNode makeVariable(DictionaryPtr dictPtr, int intVal);
    static CompiledNode makeCFunction(DictionaryPtr dictPtr, FuncPtr funcPtr);
    static CompiledNode makeWord(DictionaryPtr dictPtr);
    static CompiledNode makeUnknown();
    static SuccessOrFailure run_full_phrase(CompiledNodes& c);

    void dots();
    ExecuteResult execute(CompiledNodes::iterator it);
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
