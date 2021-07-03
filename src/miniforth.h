#ifndef __MINIFORTH_H__
#define __MINIFORTH_H__

#include "mini_stl.h"
#include "defines.h"

class StackNode;
class CompiledNode;

typedef string Word;
typedef forward_list<StackNode> StackNodes;
typedef forward_list<CompiledNode> CompiledNodes;
typedef tuple<Word, CompiledNodes> DictionaryEntry;
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

class Forth
{
    // The currently being populated dictionary key
    Word _dictionary_key;

    // ...which is only different from "" when we are compiling:
    bool _compiling;

    // Interpreter state-machine-related variables
    bool definingConstant;
    bool definingVariable;
    bool definingString;
    const char *startOfString;

public:
    // The execution stack
    static StackNodes _stack;

    // All the known words
    static DictionaryType _dict;
    // ...and how to look them up.
    DictionaryPtr lookup(const char *wrd);

    // The do/loop stack
    static LoopsStates _loopStates;

    // The if/else/then stack
    static IfStates _ifStates;

    // The number of columns to span over for the next "."
    static int _dotNumberOfDigits;
private:

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

    Optional<int> isnumber(const char * word);
    static Optional<int> needs_a_number(const __FlashStringHelper *msg);
    Optional<CompiledNode> compile_word(const char *word);
    SuccessOrFailure interpret(const char *word);
    void undoStrtok(char *word);

public:
    Forth();
    SuccessOrFailure parse_line(char *begin, char *end);
    void reset();
};

#endif
