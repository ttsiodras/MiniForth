#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "miniforth.h"
#include "helpers.h"
#include "errors.h"

EvalResult Forth::evaluate_stack_top(const char *errorMessage)
{
    if (_stack.empty()) {
        if(errorMessage)
            error(errorMessage);
        return FAILURE;
    }
    auto topVal = _stack[_stack._currentSize-1];
    _stack.pop_back();
    switch (topVal._kind){
    case StackNode::LIT:
        return EvalResult( topVal._u.intVal);
    default:
        CompiledPhrases& c = topVal._u.dictPtr->_t2;
        for(int j=0; j<c._currentSize; j++) {
            CompiledNode& node = c[j];
            if (!node.execute())
                return FAILURE;
        }
        return evaluate_stack_top(errorMessage);
    }
}

bool Forth::commonArithmetic(int& v1, int& v2, const char *msg)
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

SuccessOrFailure Forth::add(void)
{
    int v1, v2;
    if (!commonArithmetic(v1, v2, "'+' needs two arguments..."))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2+v1));
    return SUCCESS;
}

SuccessOrFailure Forth::sub(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, "'-' needs two arguments..."))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2-v1));
    return SUCCESS;
}

SuccessOrFailure Forth::mul(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, "'*' needs two arguments..."))
        return FAILURE;
    _stack.push_back(StackNode::makeNr(v2*v1));
    return SUCCESS;
}

SuccessOrFailure Forth::div(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, "'/' needs two arguments..."))
        return FAILURE;
    if (!v2)
        return error("Division by zero...");
    _stack.push_back(StackNode::makeNr(v2/v1));
    return SUCCESS;
}

SuccessOrFailure Forth::dot(void)
{
    auto ret = evaluate_stack_top("Nothing on the stack...");
    if (ret._t1 == SUCCESS) {
        dprintf("%d", ret._t2);
    }
    return ret._t1;
}

SuccessOrFailure Forth::dots(void)
{
    Serial.print(F("[ "));
    for(int i=0; i<_stack._currentSize; i++) {
        _stack[i].dots();
    }
    Serial.print(F("]\n"));
    memory_info();
    return SUCCESS;
}

SuccessOrFailure Forth::at(void)
{
    const char *errMsg = "@ needs a variable on the stack";
    if (_stack.empty())
        return error(errMsg);
    StackNode tmp = _stack[_stack._currentSize-1];
    if (StackNode::LIT == tmp._kind)
        return error(errMsg);
    CompiledPhrases& c = tmp._u.dictPtr->_t2;
    if (c._currentSize != 1)
        return error(errMsg);
    CompiledNode& node = c[0];
    if (node._kind != CompiledNode::VARIABLE)
        return error(errMsg);
    _stack.pop_back();
    _stack.push_back(StackNode::makeNr(node.getVariableValue()));
    return SUCCESS;
}

SuccessOrFailure Forth::words(void) 
{
    for(auto& word: _dict) {
        dprintf("%s ", (char *)word._t1);
    }
    Serial.print(F("\n"));
    return SUCCESS;
}

SuccessOrFailure Forth::bang(void)
{
    const char *errMsg = "! needs a variable and a value on the stack";
    if (_stack.empty())
        return error(errMsg);
    StackNode tmp = _stack[_stack._currentSize-1];
    if (StackNode::LIT == tmp._kind)
        return error(errMsg);
    CompiledPhrases& c = tmp._u.dictPtr->_t2;
    if (c._currentSize != 1)
        return error(errMsg);
    CompiledNode& node = c[0];
    if (node._kind != CompiledNode::VARIABLE)
        return error(errMsg);
    _stack.pop_back();

    // Then, compute the value
    auto ret = evaluate_stack_top("Failed to evaluate value for !...");
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
    _compiling(false)
{
    _dictionary_key[0] = '\0';
    struct {
        const char *name;
        CompiledNode::FuncPtr funcPtr;
    } c_ops[] = {
        { "+",  &Forth::add  },
        { "-",  &Forth::sub  },
        { "*",  &Forth::mul  },
        { "/",  &Forth::div  },
        { ".",  &Forth::dot  },
        { "@",  &Forth::at   },
        { "!",  &Forth::bang },
        { ".s", &Forth::dots },
        { "wrd",  &Forth::words },
    };

    for(auto cmd: c_ops) {
        Word word;
        CompiledPhrases tmp;
        SAFE_STRCPY(word, (char*)cmd.name);
        _dict.push_back(make_tuple(word, tmp));
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

Optional<CompiledNode> Forth::compile_word(const char *word)
{
    auto numericValue = isnumber(word);
    if (numericValue._t1) {
        return CompiledNode::makeLiteral(numericValue._t2);
    }
    auto it = lookup(word);
    if (!it) {
        error("Unknown word:", word);
        return FAILURE;
    }
    return CompiledNode::makeWord(it);
}

SuccessOrFailure Forth::interpret(const char *word)
{
    if (!strcmp(word, "variable")) {
        // We expect vars/constants to have a default initialization value
        if (_stack.empty())
            return error("You forgot to initialise the variable...");
        definingVariable = true;
    } else if (!strcmp(word, "constant")) {
        // We expect vars/constants to have a default initialization value
        if (_stack.empty())
            return error("You forgot to initialise the constant...");
        definingConstant = true;
    } else {
        auto numericValue = isnumber(word);
        if (numericValue._t1)
            _stack.push_back(StackNode::makeNr(numericValue._t2));
        else {
            // Must be in the dictionary
            auto ptrWord = lookup(word);
            if (!ptrWord)
                return error("No such symbol found: ", word);
            for(int j=0; j<ptrWord->_t2._currentSize; j++) {
                CompiledNode& node = ptrWord->_t2[j];
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
        if (*word == ':') {
            _compiling = true;
        } else if (*word == ';') {
            if (_compiling) {
                _compiling = false;
                _dictionary_key[0] = '\0';
                if (definingVariable)
                    return error("You didn't finish defining the variable...");
                if (definingConstant)
                    return error("You didn't finish defining the constant...");
            } else
                return error("Not in compiling mode...");
        } else {
            if (_compiling) {
                if (_dictionary_key[0] == '\0') {
                    SAFE_STRCPY(_dictionary_key, word);
                    _dict.push_back(
                        make_tuple( _dictionary_key, CompiledPhrases()));
                } else {
                    auto ret = compile_word(word);
                    if (!ret._t1)
                        return error("Failed to parse word:", word);
                    auto ptrWord = lookup(_dictionary_key);
                    ptrWord->_t2.push_back(ret._t2);
                }
            } else {
                if (definingConstant) {
                    auto ret = evaluate_stack_top(
                        "[x] Failure computing constant...");
                    if (ret._t1) {
                        // dictIdx unknown for now
                        auto c = CompiledNode::makeConstant(NULL);
                        c.setConstantValue(ret._t2);
                        CompiledPhrases tmp;
                        tmp.push_back(c);
                        SAFE_STRCPY(_dictionary_key, word);
                        _dict.push_back(make_tuple(_dictionary_key, tmp));
                        auto lastWordPtr = &*_dict.begin();
                        // Now that we know it, update the dictIdx
                        lastWordPtr->_t2[0]._u._constant._dictPtr = lastWordPtr;
                        _dictionary_key[0] = '\0';
                    }
                    definingConstant = false;
                } else if (definingVariable) {
                    auto ret = evaluate_stack_top(
                        "[x] Failure computing variable initial value...");
                    if (ret._t1) {
                        // dictIdx unknown for now
                        auto vCompiledNode = CompiledNode::makeVariable(NULL, ret._t2);
                        BoundedArray<CompiledNode, MAX_PHRASE_ENTIES> tmp;
                        tmp.push_back(vCompiledNode);
                        SAFE_STRCPY(_dictionary_key, word);
                        _dict.push_back(make_tuple(_dictionary_key, tmp));
                        auto lastWordPtr = &*_dict.begin();
                        // Now that we know it, update the dictIdx
                        lastWordPtr->_t2[0]._u._variable._dictPtr = lastWordPtr;
                        _dictionary_key[0] = '\0';
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
    return SUCCESS;
}

int CompiledNode::_memory[MEMORY_SIZE] = {0};
unsigned CompiledNode::_currentMemoryOffset = 0;
RuntimePhrases Forth::_stack;
DictionaryType Forth::_dict;
