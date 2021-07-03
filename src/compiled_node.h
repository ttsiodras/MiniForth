#ifndef __COMPILED_NODE_H__
#define __COMPILED_NODE_H__

#include "errors.h"

struct CompiledNode {

    // The kinds of Forth constructs we support
    enum CompiledNodeType {
        UNKNOWN, // Used during construction time
        LITERAL,
        STRING,
        CONSTANT,
        VARIABLE,
        C_FUNC,
        WORD
    };

    // Type used for C_FUNC callbacks
    //
    // When we execute code, we want to be able to tamper with
    // "the Program Counter". Since we use single-linked lists
    // (forward_list) of CompiledNodes, the program counter is
    // the current position in the list, during an iteration...
    // i.e. an iterator!
    // And since execution can fail - e.g. a '+' that finds
    // less values on the stack than it needs - the returned
    // result is an Optional<iterator>.
    // The returned iterator tells us where to jump in the list
    // (e.g. think of DO ... LOOP; the LOOP will return the
    // iterator to the DO, restarting the loop.
    typedef Optional<CompiledNodes::iterator> ExecuteResult;
    typedef ExecuteResult (*FuncPtr)(CompiledNodes::iterator);

    // The data of each kind.
    // I know, I know - not very C++ of me :-)
    // Remember: I built this in just one week of post-work afternoons :-)
    // And sometimes, putting all _dictPtr in the same slot
    // allows for "cheap" virtual methods :-)
    CompiledNodeType _kind;
    union UnionData {
        UnionData() {}
        struct {
            // The pointer to the dictionary entry - mostly used
            // to get the word's name (it's stored there as the
            // tuple's first field).
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

    // This runs the complete list of words inside a word.
    static SuccessOrFailure run_full_phrase(CompiledNodes& c);

    // ".S" - dump the stack out
    void dots();

    // CompiledNode, do your thing
    ExecuteResult execute(CompiledNodes::iterator it);

    void setConstantValue(int intVal);
    void setVariableValue(int intVal);
    int getVariableValue();
};

#endif
