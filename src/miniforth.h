#ifndef __MINIFORTH_H__
#define __MINIFORTH_H__

#include "mini_stl.h"
#include "defines.h"

class StackNode;
class CompiledNode;

typedef string Word;
typedef forward_list<StackNode> StackNodes;
typedef forward_list<CompiledNode> CompiledNodes;
class DictionaryEntry : private tuple<Word, CompiledNodes> {
public:
    DictionaryEntry(const Word& name, const CompiledNodes& nodes) {
        this->_t1 = name;
        this->_t2 = nodes;
    }
    const char *name() { return _t1.c_str(); }
    CompiledNodes& getCompiledNodes() { return _t2; }
};
typedef DictionaryEntry* DictionaryPtr;
typedef forward_list<DictionaryEntry> DictionaryType;

typedef struct LoopState {
    int _idxBegin;
    int _idxEnd;
    int _currentIdx;
    CompiledNodes::iterator _firstWordInLoop;
    LoopState(int begin, int end, CompiledNodes::iterator firstWordInLoop)
        :_idxBegin(begin),
         _idxEnd(end),
         _currentIdx(begin),
         _firstWordInLoop(firstWordInLoop) {}
} LoopState;
typedef forward_list<LoopState> LoopsStates;

typedef struct IfState {
    bool _lastIfTruth;
    IfState(bool b):_lastIfTruth(b) {}
    bool wasTrue() { return _lastIfTruth; }
    static bool inside_IF_body;
} IfState;
typedef forward_list<IfState> IfStates;

#include "stack_node.h"
#include "compiled_node.h"

// The proliferation of 'static' is because, frankly...
// ... there can be only one Forth! And no, the singleton pattern
// doesn't apply here, because we don't want to waste space passing
// 'this' when we don't have to.
//
// Space... the final frontier. In Arduino-land, at least :-)
//
class Forth
{
    // The currently being populated dictionary key
    static Word _dictionary_key;

    // ...which is only different from "" when we are compiling:
    static bool _compiling;
    static DictionaryPtr _wordBeingCompiled;

    // Interpreter state-machine-related variables
    static bool definingConstant;
    static bool definingVariable;
    static bool definingString;
    static const char *startOfString;

    // The words that have a C++ implementation
    typedef struct tag_BakedInCommand {
        // Naturally, the name is stored in Flash.
        const __FlashStringHelper *name;
        // What code to call when we see this word.
        CompiledNode::FuncPtr funcPtr;
    } BakedInCommand;

    // Could not define these as class-globals, because the use of "F" leads to:
    //
    //    error: statement-expressions are not allowed outside functions
    //    nor in template-argument lists
    //
    // So... workaround (see implementation for details)
    static const BakedInCommand* iterate_on_C_ops(bool reset=false);

public:
    // The execution stack
    static StackNodes _stack;

    // All the known words
    static DictionaryType _dict;
    // ...and how to look them up.
    static DictionaryPtr lookup(const char *wrd);
    // Also: a way to look up natively-implemented words
    const static BakedInCommand *lookup_C(const char *wrd);

    // The do/loop stack
    static LoopsStates _loopStates;

    // The if/else/then stack
    static IfStates _ifStates;

    // The number of columns to span over for the next "."
    static int _dotNumberOfDigits;

    static EvalResult evaluate_stack_top(const __FlashStringHelper *errorMessage);
    static bool commonArithmetic(int& v1, int& v2, const __FlashStringHelper *msg);
    static CompiledNode::ExecuteResult add(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult sub(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult mul(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult div(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult mod(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult muldiv(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult dot(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult at(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult bang(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult dots(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult CR(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult words(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult doloop(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult loop(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult loop_I(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult loop_J(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult UdotR(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult dup(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult drop(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult equal(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult greater(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult less(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult iff(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult elsee(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult then(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult swap(CompiledNodes::iterator it);
    static CompiledNode::ExecuteResult rot(CompiledNodes::iterator it);

private:
    static Optional<int> isnumber(const char * word);
    static Optional<int> needs_a_number(const __FlashStringHelper *msg);
    static Optional<CompiledNode> compile_word(const char *word);
    static SuccessOrFailure interpret(const char *word);
    static void undoStrtok(char *word);

public:
    Forth();
    static SuccessOrFailure parse_line(char *begin, char *end);
    static void reset();
};

#endif
