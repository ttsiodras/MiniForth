#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "miniforth.h"
#include "helpers.h"
#include "errors.h"

// Instantiate the template-class globals of our lists.
// See relevant comment in mini_stl.h

template<class T>
typename forward_list<T>::box *forward_list<T>::_freeList = NULL;

template<class T>
unsigned forward_list<T>::_freeListMemory = 0;

// Re-use error message space as much as possible!
const char emptyMsg[] PROGMEM = {
    "Stack is empty when it shouldn't be..."
};
__FlashStringHelper* emptyMsgFlash = (__FlashStringHelper*)emptyMsg;

// Anyone who needs to get an integer off the top of the stack, 
// will call this.
EvalResult Forth::evaluate_stack_top(const __FlashStringHelper *errorMessage)
{
    if (_stack.empty())
        return error(emptyMsgFlash, errorMessage);
    auto topVal = *_stack.begin();
    _stack.pop_front();
    switch (topVal._kind){
    case StackNode::LIT:
        // Evaluating a LITERAL node is easy: put the value on the stack
        return EvalResult(topVal._u.intVal);
    default:
        // For anything else, execute all the corresponding words...
        if (!CompiledNode::run_full_phrase(topVal._u.dictPtr->getCompiledNodes()))
            return FAILURE;
        // ...and hope that they left an integer on the top!
        return evaluate_stack_top(errorMessage);
    }
}

// Re-used from '+', '-', '*', '/' etc...
bool Forth::commonArithmetic(int& v1, int& v2, const __FlashStringHelper *msg)
{
    auto ret1 = evaluate_stack_top(msg);
    if (!ret1)
        return FAILURE;
    v1 = ret1.value();

    auto ret2 = evaluate_stack_top(msg);
    if (!ret2)
        return FAILURE;
    v2 = ret2.value();
    return SUCCESS;
}

// Re-used error message when not enough arguments are on the stack
const char arithmeticErrorMsg[] PROGMEM = {
    "Arithmetic operations need arguments that evaluate to a number."
};
__FlashStringHelper* arithmeticErrorMsgFlash = (__FlashStringHelper*)arithmeticErrorMsg;

CompiledNode::ExecuteResult Forth::add(CompiledNodes::iterator it)
{
    int v1, v2;
    if (!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2+v1));
    return it;
}

CompiledNode::ExecuteResult Forth::sub(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2-v1));
    return it;
}

CompiledNode::ExecuteResult Forth::mul(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2*v1));
    return it;
}

CompiledNode::ExecuteResult Forth::muldiv(CompiledNodes::iterator it)
{
    int v1, v2, v3;
    auto ret1 = evaluate_stack_top(arithmeticErrorMsgFlash);
    if (!ret1) return FAILURE;
    v1 = ret1.value();

    auto ret2 = evaluate_stack_top(arithmeticErrorMsgFlash);
    if (!ret2) return FAILURE;
    v2 = ret2.value();

    auto ret3 = evaluate_stack_top(arithmeticErrorMsgFlash);
    if (!ret3) return FAILURE;
    v3 = ret3.value();

    _stack.push_back(StackNode::makeNr((long(v3)*v2)/v1));
    return it;
}

CompiledNode::ExecuteResult Forth::div(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    if (!v1)
        return error(F("Division by zero..."));
    _stack.push_back(StackNode::makeNr(v2/v1));
    return it;
}

CompiledNode::ExecuteResult Forth::mod(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    if (!v1)
        return error(F("Division by zero..."));
    _stack.push_back(StackNode::makeNr(v2%v1));
    return it;
}

CompiledNode::ExecuteResult Forth::equal(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2 == v1 ? 1 : 0));
    return it;
}

CompiledNode::ExecuteResult Forth::greater(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2 > v1 ? 1 : 0));
    return it;
}

CompiledNode::ExecuteResult Forth::less(CompiledNodes::iterator it)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2 < v1 ? 1 : 0));
    return it;
}

CompiledNode::ExecuteResult Forth::dot(CompiledNodes::iterator it)
{
    auto ret = evaluate_stack_top(F("Nothing on the stack..."));
    if (!ret)
        return FAILURE;
    if (_dotNumberOfDigits) {
        // Sadly, Arduino vsnprintf doesn't support 
        // the '*' syntax in the format spec...
        // i.e. "%*d", _dotNumberOfDigits, value...
        // So count the digits we would need,
        // and emit enough spaces.
        int digits = snprintf(NULL, 0, "%d", ret.value());
        while(digits++ < _dotNumberOfDigits)
            Serial.print(" ");
        dprintf(" %d", ret.value());
        _dotNumberOfDigits = 0; // back to normal (reset from U.R)
    } else 
        dprintf(" %d", ret.value());
    return it;
}

CompiledNode::ExecuteResult Forth::CR(CompiledNodes::iterator it)
{
    dprintf("%s", "\n");
    return it;
}

// Re-used error message when not enough arguments are on the stack
const char swapErrorMsg[] PROGMEM = {
    "A SWAP depends on two items existing on the stack."
};
__FlashStringHelper* swapErrorMsgFlash = (__FlashStringHelper*)swapErrorMsg;

CompiledNode::ExecuteResult Forth::swap(CompiledNodes::iterator it)
{
    if (_stack.empty())
        return error(emptyMsgFlash, swapErrorMsg);
    auto topVal = *_stack.begin();
    _stack.pop_front();

    if (_stack.empty()) {
        _stack.push_back(topVal);
        return error(emptyMsgFlash, swapErrorMsg);
    }
    auto bottomVal = *_stack.begin();
    _stack.pop_front();

    _stack.push_back(topVal);
    _stack.push_back(bottomVal);
    return it;
}

// Re-used error message when not enough arguments are on the stack
const char rotErrorMsg[] PROGMEM = {
    "A ROT depends on three items existing on the stack."
};
__FlashStringHelper* rotErrorMsgFlash = (__FlashStringHelper*)rotErrorMsg;

CompiledNode::ExecuteResult Forth::rot(CompiledNodes::iterator it)
{
    if (_stack.empty())
        return error(emptyMsgFlash, rotErrorMsg);
    auto val3 = *_stack.begin();
    _stack.pop_front();

    if (_stack.empty()) {
        _stack.push_back(val3);
        return error(emptyMsgFlash, rotErrorMsg);
    }
    auto val2 = *_stack.begin();
    _stack.pop_front();

    if (_stack.empty()) {
        _stack.push_back(val2);
        _stack.push_back(val3);
        return error(emptyMsgFlash, rotErrorMsg);
    }
    auto val1 = *_stack.begin();
    _stack.pop_front();

    _stack.push_back(val2);
    _stack.push_back(val3);
    _stack.push_back(val1);
    return it;
}

// Re-used error message when not enough arguments are on the stack
const char loopErrorMsg[] PROGMEM = {
    "A DO depends on two arithmetic operands existing on top of the stack."
};
__FlashStringHelper* loopErrorMsgFlash = (__FlashStringHelper*)loopErrorMsg;

CompiledNode::ExecuteResult Forth::doloop(CompiledNodes::iterator it)
{
    int loopBegin, loopEnd;

    auto ret1 = evaluate_stack_top(loopErrorMsgFlash);
    if (!ret1) return FAILURE;
    loopBegin = ret1.value();

    auto ret2 = evaluate_stack_top(loopErrorMsgFlash);
    if (!ret2) return FAILURE;
    loopEnd = ret2.value();

    // When you meet a DO loop, you need to remember 
    // the current "instruction counter", because when
    // you meet the LOOP, you need to return to it!
    // So push the PC...
    CompiledNodes::iterator itFirstWordInLoop = it;
    // ...of the next instruction...
    ++itFirstWordInLoop;
    // ...on the LOOP stack.
    _loopStates.push_back(LoopState(loopBegin, loopEnd, itFirstWordInLoop));
    return it;
}

CompiledNode::ExecuteResult Forth::loop(CompiledNodes::iterator it)
{
    if (_loopStates.empty())
        return error(emptyMsgFlash, F("LOOP needs a previous DO"));
    // Time to use the information we stored when we met the DO
    auto& loopState = *_loopStates.begin();
    // Increment the loop counter...
    loopState._currentIdx++;
    // ...and if have reachede the final boundary, continue 
    // execution past the LOOP.
    if (loopState._currentIdx >= loopState._idxEnd) {
        _loopStates.pop_front();
        return it;
    } else {
        // Otherwise, jump back to the DO!
        return loopState._firstWordInLoop;
    }
}

// The IF/ELSE/THEN also needs an execution stack - to allow
// nested IFs. Read the explanation in the run_full_phrase
// method of CompiledNode.
CompiledNode::ExecuteResult Forth::iff(CompiledNodes::iterator it)
{
    auto msg = F("IF needs a number...");
    auto topVal = needs_a_number(msg);
    if (!topVal)
        return error(msg);
    _stack.pop_front();

    // Read the explanation in the run_full_phrase
    // method of CompiledNode to understand these two lines.
    _ifStates.push_back(IfState(0 != topVal.value()));
    IfState::inside_IF_body = true;
    return it;
}

CompiledNode::ExecuteResult Forth::then(CompiledNodes::iterator it)
{
    // Read the explanation in the run_full_phrase
    // method of CompiledNode to understand these two lines.
    _ifStates.pop_front();
    return it;
}

CompiledNode::ExecuteResult Forth::elsee(CompiledNodes::iterator it)
{
    // Read the explanation in the run_full_phrase
    // method of CompiledNode to understand these two lines.
    IfState::inside_IF_body = false;
    return it;
}

CompiledNode::ExecuteResult Forth::loop_I(CompiledNodes::iterator it)
{
    if (_loopStates.empty()) 
        return error(emptyMsgFlash, F("I needs a previous DO"));
    // Put the top-most counter in the LOOP stack...
    auto& loopState = *_loopStates.begin();
    // ...on the Forth stack.
    _stack.push_back(StackNode::makeNr(loopState._currentIdx));
    return it;
}

CompiledNode::ExecuteResult Forth::loop_J(CompiledNodes::iterator it)
{
    auto errMsg = F("J needs *two* previous DO");
    if (_loopStates.empty()) 
        return error(emptyMsgFlash, errMsg);
    if (_loopStates.begin()._p->_next == NULL)
        return error(emptyMsgFlash, errMsg);
    // Put the second-from-the-top-most counter in the LOOP stack...
    _stack.push_back(StackNode::makeNr(_loopStates.begin()._p->_next->_data._currentIdx));
    return it;
}

// Helper - save on Flash space by doing this in one place!
Optional<int> Forth::needs_a_number(const __FlashStringHelper *msg)
{
    if (_stack.empty())
        return error(emptyMsgFlash, msg);
    auto topVal = *_stack.begin();
    if (topVal._kind == StackNode::LIT)
        return topVal._u.intVal;
    else
        return FAILURE;
}

CompiledNode::ExecuteResult Forth::UdotR(CompiledNodes::iterator it)
{
    auto msg = F("U.R needs the number of columns");
    auto topVal = needs_a_number(msg);
    if (!topVal)
        return error(msg);
    _stack.pop_front();
    // Update the global state used by the 'dot' member
    // to 'pad' the next print with spaces.
    _dotNumberOfDigits = topVal.value();
    return it;
}

CompiledNode::ExecuteResult Forth::dup(CompiledNodes::iterator it)
{
    if (_stack.empty())
        return error(emptyMsgFlash, F("DUP needs a non-empty stack"));
    auto topVal = *_stack.begin();
    _stack.push_back(topVal);
    return it;
}

CompiledNode::ExecuteResult Forth::drop(CompiledNodes::iterator it)
{
    if (_stack.empty())
        return error(emptyMsgFlash, F("DROP` needs a non-empty stack"));
    _stack.pop_front();
    return it;
}

CompiledNode::ExecuteResult Forth::dots(CompiledNodes::iterator it)
{
    Serial.print(F("[ "));
    // This is the simplest way to reverse the order...
    // But it's also wasteful - it needs at least as much
    // room in our pool to host what currently exists
    // in our stack...
    //
    // forward_list<StackNode> swapperList;
    // for(auto& stackNode: _stack)
    //     swapperList.push_back(stackNode);
    // for(auto& stackNode: swapperList)
    //     stackNode.dots();
    // while(!swapperList.empty())
    //     swapperList.pop_front();
    //
    // Instead... a much more optimal, memory-wise, way:
    void *pEnd = NULL;
    while(true) {
        // Hunt for the LAST element
        forward_list<StackNode>::iterator it2 = _stack.begin();
        while(it2._p && it2.next() != pEnd)
            ++it2;
        if ( it2 == _stack.begin())
            break;
        it2->dots();
        // Move the LAST element to hunt for, to one step before...
        pEnd = it2._p;
    }
    // Finally, print the head element.
    if (!_stack.empty())
        _stack.begin()->dots();

    Serial.print(F("] "));

    // Print some memory stats, too.
    memory_info(
        forward_list<StackNode>::_freeListMemory +
        forward_list<CompiledNode>::_freeListMemory);
    return it;
}

CompiledNode::ExecuteResult Forth::at(CompiledNodes::iterator it)
{
    const __FlashStringHelper *errMsg = \
        F("@ needs a variable or constant on the stack");
    if (_stack.empty())
        return error(emptyMsgFlash, errMsg);
    auto tmp = *_stack.begin();
    // Constant value, e.g. $1234. Dereference it as ptr to int
    // Useful to access register space directly.
    if (StackNode::LIT == tmp._kind) {
        _stack.pop_front();
        _stack.push_back(StackNode::makeNr( *reinterpret_cast<int *>(tmp._u.intVal)));
    } else {
        CompiledNodes& c = tmp._u.dictPtr->getCompiledNodes();
        if (c.empty())
            return error(emptyMsgFlash, errMsg);
        CompiledNode& node = *c.begin();
        if (node._kind != CompiledNode::VARIABLE && node._kind != CompiledNode::CONSTANT)
            return error(errMsg);
        _stack.pop_front();
        _stack.push_back(StackNode::makeNr(node.getVariableValue()));
    }
    return it;
}

CompiledNode::ExecuteResult Forth::words(CompiledNodes::iterator it) 
{
    for(auto& word: _dict) {
        dprintf("%s ", (char *)word.name());
    }
    Serial.print(F(".\" reset\n"));
    return it;
}

CompiledNode::ExecuteResult Forth::bang(CompiledNodes::iterator it)
{
    const __FlashStringHelper *errMsg = \
        F("! needs a [variable|constant] and a value on the stack");
    if (_stack.empty())
        return error(emptyMsgFlash, errMsg);
    auto tmp = *_stack.begin();
    // Our StackNode-s can be either a LITERAL/CONSTANT,
    // ...in which case we just treat them as pointer to int...
    if (StackNode::LIT == tmp._kind) {
        _stack.pop_front();
        int *pDest = reinterpret_cast<int *>(tmp._u.intVal);
        auto ret = evaluate_stack_top(F("Failed to evaluate value for !..."));
        if (ret) {
            *pDest = ret.value();
            return it;
        }
        return FAILURE;
    } else {
        // ...or it can be something that the dictionary knows:
        // In this case, an actual VARIABLE.
        auto& c = tmp._u.dictPtr->getCompiledNodes();
        if (c.empty())
            return error(emptyMsgFlash, errMsg);
        // Since we hunt for a variable, there must be
        // such a node a the top of that DictionaryEntry's list:
        CompiledNode& node = *c.begin();
        if (node._kind != CompiledNode::VARIABLE)
            return error(errMsg);
        _stack.pop_front();

        // Then, compute the value
        auto ret = evaluate_stack_top(F("Failed to evaluate value for !..."));
        if (ret) {
            node.setVariableValue(ret.value());
            return it;
        }
        return FAILURE;
    }
}

// Perform a case-insensitive lookup for the word entered on the REPL.
DictionaryPtr Forth::lookup(const char *wrd) {
    for(auto it = _dict.begin(); it != _dict.end(); ++it) {
        if (!strcasecmp(wrd, (char *)it->name()))
             return &*it;
    }
    return NULL;
}

void Forth::reset()
{
    struct {
        const __FlashStringHelper *name;
        CompiledNode::FuncPtr funcPtr;
    } c_ops[] = {
        { F("+"),     &Forth::add    },
        { F("-"),     &Forth::sub    },
        { F("*"),     &Forth::mul    },
        { F("/"),     &Forth::div    },
        { F("MOD"),   &Forth::mod    },
        { F("*/"),    &Forth::muldiv },
        { F("="),     &Forth::equal  },
        { F(">"),     &Forth::greater},
        { F("<"),     &Forth::less   },
        { F("."),     &Forth::dot    },
        { F("@"),     &Forth::at     },
        { F("!"),     &Forth::bang   },
        { F(".S"),    &Forth::dots   },
        { F("CR"),    &Forth::CR     },
        { F("WORDS"), &Forth::words  },
        { F("DO"),    &Forth::doloop },
        { F("LOOP"),  &Forth::loop   },
        { F("I"),     &Forth::loop_I },
        { F("J"),     &Forth::loop_J },
        { F("U.R"),   &Forth::UdotR  },
        { F("DUP"),   &Forth::dup    },
        { F("DROP"),  &Forth::drop   },
        { F("IF"),    &Forth::iff    },
        { F("THEN"),  &Forth::then   },
        { F("ELSE"),  &Forth::elsee  },
        { F("SWAP"),  &Forth::swap   },
        { F("ROT"),   &Forth::rot    },
    };

    definingVariable = false;
    definingConstant = false;
    definingString = false;
    _dictionary_key.clear();

    // Remember, each list<T> instance has a globally-reused
    // freelist. Reset them all, for each one of our types.
    forward_list<StackNode>::_freeList = NULL;
    forward_list<StackNode>::_freeListMemory = 0;
    forward_list<CompiledNode>::_freeList = NULL;
    forward_list<CompiledNode>::_freeListMemory = 0;
    forward_list<LoopState>::_freeList = NULL;
    forward_list<LoopState>::_freeListMemory = 0;
    forward_list<IfState>::_freeList = NULL;
    forward_list<IfState>::_freeListMemory = 0;
    forward_list<DictionaryEntry>::_freeList = NULL;
    forward_list<DictionaryEntry>::_freeListMemory = 0;

    // The IF stack also has some more global state
    IfState::inside_IF_body = false;

    // ...as does the "." implementation...
    _dotNumberOfDigits = 0;

    // ...and all the lists...
    _stack.clear();
    _dict.clear();
    _ifStates.clear();
    _loopStates.clear();

    // ...and the CompiledNode's memory buffer...
    CompiledNode::memory_clear();

    // ...and the master Pool itself!
    Pool::clear();

    // Add all pre-built words to the dictionary
    for(auto cmd: c_ops) {
        CompiledNodes tmp;
        _dict.push_back(DictionaryEntry(Word(cmd.name), tmp));
        auto lastWordPtr = &*_dict.begin();
        lastWordPtr->getCompiledNodes().push_back(
            CompiledNode::makeCFunction(lastWordPtr, cmd.funcPtr));
    }
    Serial.println(F("\n\n================================================================"));
    Serial.println(F("                     TTSIOD Forth"));
    Serial.println(F("----------------------------------------------------------------"));
    Serial.println(F("    Type 'words' (without the quotes) to see available words."));
    Serial.println(F("=============== Maximum line length is this long ================"));

    // I lie - the MAX_LINE_LENGTH is 80 :-)
    // But the Gods of Forth are right: you must be concise!
}

Forth::Forth():
    _compiling(false),
    definingConstant(false),
    definingVariable(false),
    definingString(false),
    startOfString(NULL)
{
    // Avoid doing things in the constructor.
    // We will instead .reset() in the first Arduino setup/loop
    //
    // There's a reason: constructors of global objects run before
    // main, and in the embedded-spaces, there-be-dragons.
}

// Parses input literal numbers (including hex ones, starting with '$')
Optional<int> Forth::isnumber(const char * word)
{
    if (0 == strlen(word))
        return FAILURE;
    const char *ptrStart = word;
    char *ptrEnd;
    int base = 10;
    if (word[0] == '$') { // hex numbers
        ptrStart++;
        base = 16;
    } else if (word[0] == '%') { // binary numbers
        ptrStart++;
        base = 2;
    }
    long val = strtol(ptrStart, &ptrEnd, base);
    if (ptrEnd == &word[strlen(word)])
        return (int)val;
    return FAILURE;
}

// To print strings without wasting space to store them as we 
// interpet, just undo the strTok-placed NULLs on the input buffer!
//
// The things I do to save memory :-)
void Forth::undoStrtok(char *word)
{
    // In the intermediate words, scan in reverse to undo strtok
    // placement of terminating nulls:
    char *undo = (char *) word;
    while(*undo != '\0')
        undo--;
    while(*undo == '\0') {
        *undo = ' ';
        undo--;
    }
}

// The compiler - it builds a CompiledNode from our input word.
// See the definition of CompiledNode for details of possible
// generated outcomes.
//
// Since this can fail, we return an Optional<CompiledNode>.
Optional<CompiledNode> Forth::compile_word(const char *word)
{
    auto numericValue = isnumber(word);
    if (!definingString && !strcmp(word, ".\"")) {
        definingString = true;
        startOfString = NULL;
        // Strings start their lives as UNKNOWN kind.
        return CompiledNode::makeUnknown();
    } else if (definingString) {
        if (!startOfString) {
            // The first word we see after a ." 
            // is the beginning of our string. Mark it.
            startOfString = word;
            // not used, just continue
            return CompiledNode::makeUnknown();
        } else if (!strcmp(word, "\"")) {
            // The string just concluded!
            definingString = false;
            // Build a copy of it into a CompiledNode
            return CompiledNode::makeString(startOfString);
        } else {
            // To avoid wasting space, just undo the
            // strTok-placed NULLs on the input buffer
            undoStrtok((char *)word);
            // not used, just continue
            return CompiledNode::makeUnknown();
        }
    } else if (numericValue) {
        return CompiledNode::makeLiteral(numericValue.value());
    } else {
        auto it = lookup(word);
        if (!it) {
            error(F("Unknown word:"), word);
            return FAILURE;
        }
        // Optimization: We don't want to insert single-C-function
        // words in the list of CompiledNodes as WORD kinds.
        // We want them to appear "naked", as C_FUNC kinds.
        auto& c = it->getCompiledNodes();
        auto itFirstNode = c.begin();
        CompiledNode& firstNode = *itFirstNode;
        ++itFirstNode;
        bool hasMoreThanOneNodes = c.end() != itFirstNode;
        if (firstNode._kind == CompiledNode::C_FUNC && !hasMoreThanOneNodes)
            return CompiledNode::makeCFunction(it, firstNode._u._function._funcPtr);
        // Otherwise, build a normal WORD.
        return CompiledNode::makeWord(it);
    }
}

SuccessOrFailure Forth::interpret(const char *word)
{
    if (definingString) {
        if (!startOfString)
            startOfString = word;
        else if (!strcmp(word, "\"")) {
            definingString = false;
            dprintf("%s",  startOfString);
        } else {
            // To avoid wasting space, just undo the
            // strTok-placed NULLs on the input buffer
            undoStrtok((char *)word);
        }
    } else if (!definingVariable && !strcmp(word, "variable")) {
        // We expect vars/constants to have a default initialization value
        if (_stack.empty())
            return error(F("You forgot to initialise the variable..."));
        definingVariable = true;
    } else if (!definingConstant && !strcmp(word, "constant")) {
        // We expect vars/constants to have a default initialization value
        if (_stack.empty())
            return error(F("You forgot to initialise the constant..."));
        definingConstant = true;
    } else if (!definingString && !strcmp(word, ".\"")) {
        definingString = true;
        startOfString = NULL;
    } else {
        // if we are not defining a string, a constant or a variable,
        auto numericValue = isnumber(word);
        if (numericValue)
            // then we are either a number...
            _stack.push_back(StackNode::makeNr(numericValue.value()));
        else {
            // ...or we must already exist in the dictionary:
            auto ptrWord = lookup(word);
            if (!ptrWord)
                return error(F("No such symbol found: "), word);
            if (!CompiledNode::run_full_phrase(ptrWord->getCompiledNodes()))
                return FAILURE;
        }
    }
    return SUCCESS;
}

// Set just once and re-used from global space
const char resetCmd[] PROGMEM = { "reset" };

SuccessOrFailure Forth::parse_line(char *begin, char *end)
{
    const char *word=begin;
    static const char *delim = " \n\r";
    
    // Split input line into words.
    word = strtok(begin, delim);
    while(word) {
        while(word<end && isspace(*word))
            word++;

        // No more data?
        if (word == end)
            break;

        // Parse word
        if (*word == '\\') {
            // Forth comments - ignore them.
            break;
        } else if (*word == ':' && *(word+1) == '\0' && !_compiling) {
            _compiling = true;
        } else if (*word == ';' && *(word+1) == '\0' && _compiling) {
            _compiling = false;
            auto ptrWord = lookup(_dictionary_key);
            _dictionary_key.clear();
            if (definingVariable)
                return error(F("You didn't finish defining the variable..."));
            if (definingConstant)
                return error(F("You didn't finish defining the constant..."));
            if (definingString)
                return error(F("You didn't finish defining the string! Enter the missing quote."));
            // We need to reverse the order of words, since we 'push_back'-ed them along...
            forward_list<CompiledNode> swapperList;
            for(auto& compNode1: ptrWord->getCompiledNodes())
                swapperList.push_back(compNode1);
            while(!ptrWord->getCompiledNodes().empty())
                ptrWord->getCompiledNodes().pop_front();
            ptrWord->getCompiledNodes() = swapperList;
        } else {
            if (_compiling) {
                if (_dictionary_key.empty()) {
                    // The first word we see after ':' is the new word being defined
                    _dictionary_key = string(word);
                    // Make a new entry in the dictionary; for now, with 
                    // an empty list of CompiledNode-s.
                    _dict.push_back(
                        DictionaryEntry( _dictionary_key, CompiledNodes()));
                } else {
                    // Any word after the first one, we compile it into
                    // a CompiledNode instance:
                    auto ret = compile_word(word);
                    if (!ret)
                        return error(F("Failed to parse word:"), word);
                    // If we are creating a string, then there's the possibility
                    // of a compile_word call that didn't do anything 
                    // (at the '."' stage). In that case, the dummy
                    // returned CompiledNode is of type UNKNOWN - ignore it.
                    if (ret.value()._kind != CompiledNode::UNKNOWN) {
                        // Otherwise, look it up, and add it to the list
                        // of our CompiledNode-s!
                        auto ptrWord = lookup(_dictionary_key);
                        ptrWord->getCompiledNodes().push_back(ret.value());
                    }
                }
            } else {
                if (definingConstant) {
                    auto ret = evaluate_stack_top(
                        F("[x] Failure computing constant..."));
                    if (ret) {
                        // We don't yet know the DictionaryEntry...
                        auto c = CompiledNode::makeConstant(NULL);
                        c.setConstantValue(ret.value());
                        CompiledNodes tmp;
                        tmp.push_back(c);
                        _dictionary_key = string(word);
                        _dict.push_back(DictionaryEntry(_dictionary_key, tmp));
                        // ...but now we do!
                        auto lastWordPtr = &*_dict.begin();
                        // ..so update the top-most entry in the dictionary.
                        // to set its _dictPtr properly:
                        lastWordPtr->getCompiledNodes().begin()->_u._constant._dictPtr = lastWordPtr;
                        _dictionary_key.clear();
                    }
                    definingConstant = false;
                } else if (definingVariable) {
                    auto ret = evaluate_stack_top(
                        F("[x] Failure computing variable initial value..."));
                    if (ret) {
                        // We don't yet know the DictionaryEntry...
                        auto vCompiledNode = CompiledNode::makeVariable(NULL, ret.value());
                        CompiledNodes tmp;
                        tmp.push_back(vCompiledNode);
                        _dictionary_key = string(word);
                        _dict.push_back(DictionaryEntry(_dictionary_key, tmp));
                        // ...but now we do!
                        auto lastWordPtr = &*_dict.begin();
                        // ..so update the top-most entry in the dictionary.
                        // to set its _dictPtr properly:
                        lastWordPtr->getCompiledNodes().begin()->_u._variable._dictPtr = lastWordPtr;
                        _dictionary_key.clear();
                    }
                    definingVariable = false;
                } else {
                    if (!definingString && !strcasecmp_P(word, resetCmd)) {
                        reset();
                        break;
                    }
                    if (!interpret(word))
                        break;
                }
            }
        }
        word = strtok(NULL, delim);
    }
    if (definingVariable)
        return error(F("You didn't finish defining the variable..."));
    if (definingConstant)
        return error(F("You didn't finish defining the constant..."));
    if (definingString)
        return error(F("You didn't finish defining the string! Enter the missing quote."));
    if (_compiling)
        Serial.println(F("You didn't finish defining the word! Don't forget the ending ';'"));
    return SUCCESS;
}

// Define all class-globals (i.e. static-s)
int CompiledNode::_memory[MEMORY_SIZE] = {0};
unsigned CompiledNode::_currentMemoryOffset = 0;
StackNodes Forth::_stack;
DictionaryType Forth::_dict;
LoopsStates Forth::_loopStates;
IfStates Forth::_ifStates;
bool IfState::inside_IF_body = false;
int Forth::_dotNumberOfDigits = 0;
