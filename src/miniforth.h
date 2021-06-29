#ifndef __MINIFORTH_H__
#define __MINIFORTH_H__

#ifndef __x86_64
#include "ArduinoSTL.h"
#endif

#include <list>

#include "mini_stl.h"
#include "defines.h"

class StackNode;
class CompiledNode;

typedef BoundedString<MAX_NAME_LENGTH> Word;
typedef std::list<StackNode> RuntimePhrases;
typedef std::list<CompiledNode> CompiledPhrases;
typedef tuple<Word, CompiledPhrases> DictionaryEntry;
typedef std::list<DictionaryEntry> DictionaryType;
typedef DictionaryEntry* DictionaryPtr;

#include "stack_node.h"
#include "compiled_node.h"

class Forth
{
    // The currently being populated dictionary key
    Word _dictionary_key;

    // ...which is only different from "" when we are compiling:
    bool _compiling = false;

    // Interpreter state-machine-related variables
    bool definingConstant = false;
    bool definingVariable = false;

public:
    // The execution stack
    static RuntimePhrases _stack;

    // All the known words
    static DictionaryType _dict;
    // ...and how to look them up.
    DictionaryPtr lookup(const char *wrd);
private:

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
    Optional<int> isnumber(const char * word);
    Optional<CompiledNode> compile_word(const char *word);
    SuccessOrFailure interpret(const char *word);

public:
    Forth();
    static SuccessOrFailure words(void) ;
    SuccessOrFailure parse_line(char *begin, char *end);
};

#endif
