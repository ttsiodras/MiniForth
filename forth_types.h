#ifndef __NODE_H__
#define __NODE_H__

#include <ArduinoSTL.h>
#include <list>

#include <string.h>

#include "defines.h"
#include "helpers.h"
#include "mini_stl.h"
#include "dassert.h"

// The kinds of Forth constructs we support
enum NodeType {
    UNKNOWN,
    LITERAL,
    CONSTANT,
    VARIABLE,
    C_FUNC,
    WORD
};

typedef tuple<SuccessOrFailure, int> EvalResult;

class Node;
class NrOrIdx;

typedef BoundedString<MAX_NAME_LENGTH> Word;
typedef std::list<NrOrIdx> RuntimePhrases;
typedef std::list<Node> CompiledPhrases;
typedef tuple<Word, CompiledPhrases> DictionaryEntry;
typedef std::list<DictionaryEntry> DictionaryType;
typedef DictionaryEntry* DictionaryPtr;

extern RuntimePhrases _stack;
extern DictionaryType _dict;

// The run-time stack
extern DictionaryPtr lookup(const char *wrd);

// Type used for C_FUNC callbacks
typedef SuccessOrFailure (*FuncPtr)();

class Node {
public:
    NodeType _kind;
    union UnionData {
        UnionData() {}
        struct {
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

    Node():_kind(UNKNOWN) {
        memset(&_u, 0, sizeof(_u));
    }
    Node(const Node& rhs) {
        _kind = rhs._kind;
        _u = rhs._u;
    }
    static Node makeLiteral(int intVal);
    static Node makeConstant(DictionaryPtr dictPtr);
    static Node makeVariable(DictionaryPtr dictPtr, int intVal);
    static Node makeCFunction(DictionaryPtr dictPtr, FuncPtr funcPtr);
    static Node makeWord(DictionaryPtr dictPtr);

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

class NrOrIdx {
public:
    NrOrIdx():_kind(UNKNOWN) { _u.intVal = -1; }
    NrOrIdx(const NrOrIdx& rhs) {
        _kind = rhs._kind;
        _u = rhs._u;
    }
    enum StackNodeType { UNKNOWN, LIT, IDX } _kind;
    union StackData {
        StackData() { intVal = -1; }
        int intVal;
        DictionaryPtr dictPtr;
    } _u;
    static NrOrIdx makeNr(int intVal) {
        NrOrIdx tmp;
        tmp._kind = LIT;
        tmp._u.intVal = intVal;
        return tmp;
    }
    static NrOrIdx makePtr(DictionaryPtr dictPtr) {
        NrOrIdx tmp;
        tmp._kind = IDX;
        tmp._u.dictPtr = dictPtr;
        return tmp;
    }
    bool isLiteral() { return _kind == LIT; }
    int getLiteralValue() {
        DASSERT(isLiteral());
        return _u.intVal;
    }
    DictionaryPtr getDictionaryPtr() {
        DASSERT(!isLiteral());
        return _u.dictPtr;
    }
    void dots() {
        if (_kind == NrOrIdx::LIT)
            dprintf("%d ", _u.intVal);
        else
            dprintf("%s ", (char *) _u.dictPtr->_t1);
    }
};

class Forth {

    // The currently being populated dictionary key
    Word _dictionary_key;

    // ...which is only different from "" when we are compiling:
    bool _compiling = false;

    // Interpreter state-machine-related variables
    bool definingConstant = false;
    bool definingVariable = false;

    static EvalResult evaluate_stack_top(const char *errorMessage);
    static bool commonArithmetic(int& v1, int& v2, const char *msg);
    static SuccessOrFailure add(void);
    static SuccessOrFailure sub(void);
    static SuccessOrFailure mul(void);
    static SuccessOrFailure div(void);
    static SuccessOrFailure dot(void);
    static SuccessOrFailure dots(void);
    static SuccessOrFailure at(void);
    static SuccessOrFailure bang(void);
    tuple<SuccessOrFailure, int> isnumber(const char * word);
    tuple<SuccessOrFailure,Node> compile_word(const char *word);
    SuccessOrFailure interpret(const char *word);

public:
    Forth();
    static SuccessOrFailure words(void) ;
    SuccessOrFailure parse_line(char *begin, char *end);
};

#endif
