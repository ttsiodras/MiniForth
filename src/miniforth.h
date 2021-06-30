#ifndef __MINIFORTH_H__
#define __MINIFORTH_H__

#include "mini_stl.h"
#include "defines.h"

class StackNode;
class CompiledNode;

typedef string Word;
typedef forward_list<StackNode> RuntimePhrases;
typedef forward_list<CompiledNode> CompiledPhrases;
typedef tuple<Word, CompiledPhrases> DictionaryEntry;
typedef DictionaryEntry* DictionaryPtr;
typedef forward_list<DictionaryEntry> DictionaryType;

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
    static RuntimePhrases _stack;

    // All the known words
    static DictionaryType _dict;
    // ...and how to look them up.
    DictionaryPtr lookup(const char *wrd);
private:

    static EvalResult evaluate_stack_top(const __FlashStringHelper *errorMessage);
    static bool commonArithmetic(int& v1, int& v2, const __FlashStringHelper *msg);
    static SuccessOrFailure add(void);
    static SuccessOrFailure sub(void);
    static SuccessOrFailure mul(void);
    static SuccessOrFailure muldiv(void);
    static SuccessOrFailure div(void);
    static SuccessOrFailure dot(void);
    static SuccessOrFailure dots(void);
    static SuccessOrFailure at(void);
    static SuccessOrFailure bang(void);
    static SuccessOrFailure CR(void);
    Optional<int> isnumber(const char * word);
    Optional<CompiledNode> compile_word(const char *word);
    SuccessOrFailure interpret(const char *word);
    void undoStrtok(char *word);

public:
    Forth();
    static SuccessOrFailure words(void) ;
    SuccessOrFailure parse_line(char *begin, char *end);
};

#endif
