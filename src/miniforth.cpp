#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "miniforth.h"
#include "helpers.h"
#include "errors.h"

template<class T>
typename forward_list<T>::box *forward_list<T>::_freeList = NULL;

template<class T>
unsigned forward_list<T>::_freeListMemory = 0;

EvalResult Forth::evaluate_stack_top(const __FlashStringHelper *errorMessage)
{
    if (_stack.empty()) {
        return error(errorMessage);
    }
    auto topVal = *_stack.begin();
    _stack.pop_front();
    switch (topVal._kind){
    case StackNode::LIT:
        return EvalResult(topVal._u.intVal);
    default:
        CompiledPhrases& c = topVal._u.dictPtr->_t2;
        for(auto& node: c) {
            if (!node.execute())
                return FAILURE;
        }
        return evaluate_stack_top(errorMessage);
    }
}

bool Forth::commonArithmetic(int& v1, int& v2, const __FlashStringHelper *msg)
{
    auto ret1 = evaluate_stack_top(msg);
    if (!ret1._t1)
        return FAILURE;
    v1 = ret1._t2;

    auto ret2 = evaluate_stack_top(msg);
    if (!ret2._t2)
        return FAILURE;
    v2 = ret2._t2;
    return SUCCESS;
}

const char arithmeticErrorMsg[] PROGMEM = {
    "Arithmetic operations:\n\t'+' '-' '*' '/' '*/'\n...need arguments that evaluate to a number."
};
__FlashStringHelper* arithmeticErrorMsgFlash = (__FlashStringHelper*)arithmeticErrorMsg;

SuccessOrFailure Forth::add(void)
{
    int v1, v2;
    if (!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2+v1));
    return SUCCESS;
}

SuccessOrFailure Forth::sub(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2-v1));
    return SUCCESS;
}

SuccessOrFailure Forth::mul(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2*v1));
    return SUCCESS;
}

SuccessOrFailure Forth::muldiv(void)
{
    int v1, v2, v3;
    auto ret1 = evaluate_stack_top(arithmeticErrorMsgFlash);
    if (!ret1._t1) return FAILURE;
    v1 = ret1._t2;

    auto ret2 = evaluate_stack_top(arithmeticErrorMsgFlash);
    if (!ret2._t2) return FAILURE;
    v2 = ret2._t2;

    auto ret3 = evaluate_stack_top(arithmeticErrorMsgFlash);
    if (!ret3._t2) return FAILURE;
    v3 = ret3._t2;

    _stack.push_back(StackNode::makeNr((long(v3)*v2)/v1));
    return SUCCESS;
}

SuccessOrFailure Forth::div(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, arithmeticErrorMsgFlash))
        return FAILURE;
    if (!v2)
        return error(F("Division by zero..."));
    _stack.push_back(StackNode::makeNr(v2/v1));
    return SUCCESS;
}

SuccessOrFailure Forth::dot(void)
{
    auto ret = evaluate_stack_top(F("Nothing on the stack..."));
    if (ret._t1 == SUCCESS) {
        dprintf("%d", ret._t2);
    }
    return ret._t1;
}

SuccessOrFailure Forth::CR(void)
{
    dprintf("%s", "\n");
    return SUCCESS;
}

SuccessOrFailure Forth::dots(void)
{
    Serial.print(F("[ "));
    forward_list<StackNode> swapperList;
    for(auto& stackNode: _stack)
        swapperList.push_back(stackNode);
    for(auto& stackNode: swapperList)
        stackNode.dots();
    while(!swapperList.empty())
        swapperList.pop_front();
    Serial.print(F("]\n"));
    memory_info(
        forward_list<StackNode>::_freeListMemory +
        forward_list<CompiledNode>::_freeListMemory);
    return SUCCESS;
}

SuccessOrFailure Forth::at(void)
{
    const __FlashStringHelper *errMsg = \
        F("@ needs a variable on the stack");
    if (_stack.empty())
        return error(errMsg);
    auto tmp = *_stack.begin();
    if (StackNode::LIT == tmp._kind)
        return error(errMsg);
    CompiledPhrases& c = tmp._u.dictPtr->_t2;
    if (c.empty())
        return error(errMsg);
    CompiledNode& node = *c.begin();
    if (node._kind != CompiledNode::VARIABLE)
        return error(errMsg);
    _stack.pop_front();
    _stack.push_back(StackNode::makeNr(node.getVariableValue()));
    return SUCCESS;
}

SuccessOrFailure Forth::words(void) 
{
    for(auto& word: _dict) {
        dprintf("%s ", (char *)word._t1);
    }
    Serial.print(F(".\"\n"));
    return SUCCESS;
}

SuccessOrFailure Forth::bang(void)
{
    const __FlashStringHelper *errMsg = \
        F("! needs a variable and a value on the stack");
    if (_stack.empty())
        return error(errMsg);
    auto tmp = *_stack.begin();
    if (StackNode::LIT == tmp._kind)
        return error(errMsg);
    CompiledPhrases& c = tmp._u.dictPtr->_t2;
    if (c.empty())
        return error(errMsg);
    CompiledNode& node = *c.begin();
    if (node._kind != CompiledNode::VARIABLE)
        return error(errMsg);
    _stack.pop_front();

    // Then, compute the value
    auto ret = evaluate_stack_top(F("Failed to evaluate value for !..."));
    if (ret._t1) {
        node.setVariableValue(ret._t2);
        return SUCCESS;
    }
    return FAILURE;
}

DictionaryPtr Forth::lookup(const char *wrd) {
    //dprintf("Lookup %s, against...\n", wrd);
    for(auto it = _dict.begin(); it != _dict.end(); ++it) {
        //dprintf("Comparing with %s...\n", (char *) it->_t1);
        if (!strcmp(wrd, (char *)it->_t1))
             return &*it;
    }
    return NULL;
}

Forth::Forth():
    _compiling(false),
    definingConstant(false),
    definingVariable(false),
    definingString(false),
    startOfString(NULL)
{
    _dictionary_key.clear();
    struct {
        const __FlashStringHelper *name;
        CompiledNode::FuncPtr funcPtr;
    } c_ops[] = {
        { F("+"),     &Forth::add    },
        { F("-"),     &Forth::sub    },
        { F("*"),     &Forth::mul    },
        { F("/"),     &Forth::div    },
        { F("*/"),    &Forth::muldiv },
        { F("."),     &Forth::dot    },
        { F("@"),     &Forth::at     },
        { F("!"),     &Forth::bang   },
        { F(".s"),    &Forth::dots   },
        { F("CR"),    &Forth::CR     },
        { F("words"), &Forth::words  },
    };

    for(auto cmd: c_ops) {
        CompiledPhrases tmp;
        _dict.push_back(make_tuple(Word(cmd.name), tmp));
        auto lastWordPtr = &*_dict.begin();
        lastWordPtr->_t2.push_back(
            CompiledNode::makeCFunction(lastWordPtr, cmd.funcPtr));
    }
}

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
    }
    long val = strtol(ptrStart, &ptrEnd, base);
    if (ptrEnd == &word[strlen(word)])
        return (int)val;
    return FAILURE;
}

void Forth::undoStrtok(char *word)
{
    // In the intermediate words, scan in reverse to undo strtok
    // placement of terminating nulls!
    char *undo = (char *) word;
    while(*undo != '\0')
        undo--;
    while(*undo == '\0') {
        *undo = ' ';
        undo--;
    }
}

Optional<CompiledNode> Forth::compile_word(const char *word)
{
    auto numericValue = isnumber(word);
    if (!definingString && !strcmp(word, ".\"")) {
        definingString = true;
        startOfString = NULL;
        return CompiledNode::makeUnknown();
    } else if (definingString) {
        if (!startOfString) {
            startOfString = word;
            return CompiledNode::makeUnknown();
        } else if (!strcmp(word, "\"")) {
            definingString = false;
            return CompiledNode::makeString(startOfString);
        } else {
            undoStrtok((char *)word);
            return CompiledNode::makeUnknown();
        }
    } else if (numericValue._t1) {
        return CompiledNode::makeLiteral(numericValue._t2);
    } else {
        auto it = lookup(word);
        if (!it) {
            error(F("Unknown word:"), word);
            return FAILURE;
        }
        return CompiledNode::makeWord(it);
    }
}

SuccessOrFailure Forth::interpret(const char *word)
{
    if (!definingVariable && !strcmp(word, "variable")) {
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
    } else if (definingString) {
        if (!startOfString)
            startOfString = word;
        else if (!strcmp(word, "\"")) {
            definingString = false;
            dprintf("%s",  startOfString);
        } else
            undoStrtok((char *)word);
    } else {
        auto numericValue = isnumber(word);
        if (numericValue._t1)
            _stack.push_back(StackNode::makeNr(numericValue._t2));
        else {
            // Must be in the dictionary
            auto ptrWord = lookup(word);
            if (!ptrWord)
                return error(F("No such symbol found: "), word);
            for(auto& node: ptrWord->_t2) {
                if (!node.execute())
                    return FAILURE;
            }
        }
    }
    return SUCCESS;
}

SuccessOrFailure Forth::parse_line(char *begin, char *end)
{
    const char *word=begin;
    static const char *delim = " \n\r";
    
    word = strtok(begin, delim);
    while(word) {
        while(word<end && isspace(*word))
            word++;

        // No more data?
        if (word == end)
            break;

        // Parse word
        if (*word == ':' && *(word+1) == '\0') {
            _compiling = true;
        } else if (*word == ';') {
            if (_compiling) {
                _compiling = false;
                auto ptrWord = lookup(_dictionary_key);
                _dictionary_key.clear();
                if (definingVariable)
                    return error(F("You didn't finish defining the variable..."));
                if (definingConstant)
                    return error(F("You didn't finish defining the constant..."));
                if (definingString)
                    return error(F("You didn't finish defining the string! Enter the missing quote."));
                // We need to reverse the order of words in the word we just defined
                forward_list<CompiledNode> swapperList;
                for(auto& compNode1: ptrWord->_t2) swapperList.push_back(compNode1);
                while(!ptrWord->_t2.empty()) ptrWord->_t2.pop_front();
                ptrWord->_t2 = swapperList;
            } else
                return error(F("Not in compiling mode..."));
        } else {
            if (_compiling) {
                if (_dictionary_key.empty()) {
                    _dictionary_key = string(word);
                    _dict.push_back(
                        make_tuple( _dictionary_key, CompiledPhrases()));
                } else {
                    auto ret = compile_word(word);
                    if (!ret._t1)
                        return error(F("Failed to parse word:"), word);
                    // If we are creating a string, then there's the possibility
                    // of a compile_word call that didn't do anything 
                    // (at the '."' stage):
                    if (ret._t2._kind != CompiledNode::UNKNOWN) {
                        auto ptrWord = lookup(_dictionary_key);
                        ptrWord->_t2.push_back(ret._t2);
                    }
                }
            } else {
                if (definingConstant) {
                    auto ret = evaluate_stack_top(
                        F("[x] Failure computing constant..."));
                    if (ret._t1) {
                        // dictIdx unknown for now
                        auto c = CompiledNode::makeConstant(NULL);
                        c.setConstantValue(ret._t2);
                        CompiledPhrases tmp;
                        tmp.push_back(c);
                        _dictionary_key = string(word);
                        _dict.push_back(make_tuple(_dictionary_key, tmp));
                        auto lastWordPtr = &*_dict.begin();
                        // Now that we know it, update the dictIdx
                        lastWordPtr->_t2.begin()->_u._constant._dictPtr = lastWordPtr;
                        _dictionary_key.clear();
                    }
                    definingConstant = false;
                } else if (definingVariable) {
                    auto ret = evaluate_stack_top(
                        F("[x] Failure computing variable initial value..."));
                    if (ret._t1) {
                        // dictIdx unknown for now
                        auto vCompiledNode = CompiledNode::makeVariable(NULL, ret._t2);
                        CompiledPhrases tmp;
                        tmp.push_back(vCompiledNode);
                        _dictionary_key = string(word);
                        _dict.push_back(make_tuple(_dictionary_key, tmp));
                        auto lastWordPtr = &*_dict.begin();
                        // Now that we know it, update the dictIdx
                        lastWordPtr->_t2.begin()->_u._variable._dictPtr = lastWordPtr;
                        _dictionary_key.clear();
                    }
                    definingVariable = false;
                } else {
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

int CompiledNode::_memory[MEMORY_SIZE] = {0};
unsigned CompiledNode::_currentMemoryOffset = 0;
RuntimePhrases Forth::_stack;
DictionaryType Forth::_dict;
