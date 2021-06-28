#define DEFINE_GLOBALS

#include <ArduinoSTL.h>

#include <list>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "forth_types.h"

EvalResult Forth::evaluate_stack_top(const char *errorMessage)
{
    if (_stack.empty()) {
        if(errorMessage)
            error(errorMessage);
        return make_tuple(FAILURE, -1);
    }
    auto topVal = *_stack.rbegin();
    _stack.pop_back();
    switch (topVal._kind){
    case NrOrIdx::LIT:
        return make_tuple(SUCCESS, topVal._u.intVal);
    default:
        CompiledPhrases& c = topVal._u.dictPtr->_t2;
        for(auto& node: c) {
            if (!node.execute())
                return make_tuple(FAILURE, -1);
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
    _stack.push_back(NrOrIdx::makeNr(v2+v1));
    return SUCCESS;
}

SuccessOrFailure Forth::sub(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, "'-' needs two arguments..."))
        return FAILURE;
    _stack.push_back(NrOrIdx::makeNr(v2-v1));
    return SUCCESS;
}

SuccessOrFailure Forth::mul(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, "'*' needs two arguments..."))
        return FAILURE;
    _stack.push_back(NrOrIdx::makeNr(v2*v1));
    return SUCCESS;
}

SuccessOrFailure Forth::div(void)
{
    int v1, v2;
    if(!commonArithmetic(v1, v2, "'/' needs two arguments..."))
        return FAILURE;
    if (!v2)
        return error("Division by zero...");
    _stack.push_back(NrOrIdx::makeNr(v2/v1));
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
    dprintf("[ ");
    for(auto& s: _stack) {
        s.dots();
    }
    dprintf("]\n");
    //memory_info();
    return SUCCESS;
}

SuccessOrFailure Forth::at(void)
{
    const char *errMsg = "@ needs a variable on the stack";
    if (_stack.empty())
        return error(errMsg);
    NrOrIdx tmp = *_stack.rbegin();
    if (NrOrIdx::LIT == tmp._kind)
        return error(errMsg);
    CompiledPhrases& c = tmp._u.dictPtr->_t2;
    if (c.empty())
        return error(errMsg);
    CompiledPhrases::iterator pVariableNode = c.begin();
    while (pVariableNode->_kind == WORD)
        pVariableNode = pVariableNode->_u._word._dictPtr->_t2.begin();
    if (pVariableNode->_kind != VARIABLE)
        return error(errMsg);
    _stack.pop_back();
    _stack.push_back(NrOrIdx::makeNr(pVariableNode->getVariableValue()));
    return SUCCESS;
}

SuccessOrFailure Forth::words(void) 
{
    for(auto& tup: _dict) {
        dprintf("%s ", (char *)tup._t1);
    }
    dprintf("\n");
    return SUCCESS;
}

SuccessOrFailure Forth::bang(void)
{
    const char *errMsg = "! needs a variable and a value on the stack";
    if (_stack.empty())
        return error(errMsg);
    NrOrIdx tmp = *_stack.rbegin();
    if (NrOrIdx::LIT == tmp._kind)
        return error(errMsg);
    CompiledPhrases& c = tmp._u.dictPtr->_t2;
    if (c.empty())
        return error(errMsg);
    CompiledPhrases::iterator pVariableNode = c.begin();
    while (pVariableNode->_kind == WORD)
        pVariableNode = pVariableNode->_u._word._dictPtr->_t2.begin();
    if (pVariableNode->_kind != VARIABLE)
        return error(errMsg);
    _stack.pop_back();

    // Then, compute the value
    auto ret = evaluate_stack_top("Failed to evaluate value for !...");
    if (ret._t1) {
        pVariableNode->setVariableValue(ret._t2);
        return SUCCESS;
    }
    return FAILURE;
}

DictionaryPtr lookup(const char *wrd) {
    //dprintf("Will look inside %d words...\n", _dict._currentSize);
    for(auto p=_dict.rbegin(); p!=_dict.rend(); p++) {
        // dprintf("Comparing '%s' against '%s'\n", 
        //         wrd, (char *)_dict[i]._t1);
        if (!strcmp(wrd, (char *)p->_t1))
             return &*p;
    }
    return NULL;
}

Forth::Forth():
    _compiling(false)
{
    _dictionary_key[0] = '\0';
    struct {
        const char *name;
        FuncPtr funcPtr;
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
        auto lastWordPtr = _dict.rbegin();
        lastWordPtr->_t2.push_back(
            Node::makeCFunction(&*lastWordPtr, cmd.funcPtr));
        // dprintf("Added: %s\n", (char*) _dict[lastWordIdx]._t1);
    }
}

tuple<SuccessOrFailure, int> Forth::isnumber(const char * word)
{
    if (0 == strlen(word))
        return make_tuple(FAILURE, -1);
    const char *ptrStart = word;
    char *ptrEnd;
    int base = 10;
    if (word[0] == '$') { // hex numbers
        ptrStart++;
        base = 16;
    }
    long val = strtol(ptrStart, &ptrEnd, base);
    if (ptrEnd == &word[strlen(word)])
        return make_tuple(SUCCESS, (int)val);
    return make_tuple(FAILURE, -1);
}

tuple<SuccessOrFailure,Node> Forth::compile_word(const char *word)
{
    auto numericValue = isnumber(word);
    if (numericValue._t1)
        return make_tuple(SUCCESS, Node::makeLiteral(numericValue._t2));
    auto it = lookup(word);
    if (!it) {
        error("Unknown word:", word);
        return make_tuple(FAILURE, Node::makeLiteral(0));
    }
    auto t = make_tuple(SUCCESS, Node::makeWord(it));
    return t;
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
            _stack.push_back(NrOrIdx::makeNr(numericValue._t2));
        else {
            // Must be in the dictionary
            auto p = lookup(word);
            if (!p)
                return error("No such symbol found: ", word);
            // dprintf("found: '%s'\n", _dict[i]._t1);
            for(auto& node: p->_t2) {
                //node.id();
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
                    _dict.push_back(make_tuple(_dictionary_key, CompiledPhrases()));
                } else {
                    auto ret = compile_word(word);
                    if (!ret._t1)
                        return error("Failed to parse word:", word);
                    auto p = lookup(_dictionary_key);
                    p->_t2.push_back(ret._t2);
                }
            } else {
                if (definingConstant) {
                    auto ret = evaluate_stack_top(
                        "[x] Failure computing constant...");
                    if (ret._t1) {
                        // dictPtr unknown for now, use current end
                        auto c = Node::makeConstant(NULL);
                        c.setConstantValue(ret._t2);
                        CompiledPhrases tmp;
                        tmp.push_back(c);
                        SAFE_STRCPY(_dictionary_key, word);
                        _dict.push_back(make_tuple(_dictionary_key, tmp));
                        DictionaryPtr lastWordPtr = &*_dict.rbegin();
                        // Now that we know it, update the dictIdx
                        lastWordPtr->_t2.begin()->_u._constant._dictPtr = lastWordPtr;
                        _dictionary_key[0] = '\0';
                    }
                    definingConstant = false;
                } else if (definingVariable) {
                    auto ret = evaluate_stack_top(
                        "[x] Failure computing variable initial value...");
                    if (ret._t1) {
                        // dictIdx unknown for now
                        auto vNode = Node::makeVariable(NULL, ret._t2);
                        std::list<Node> tmp;
                        tmp.push_back(vNode);
                        SAFE_STRCPY(_dictionary_key, word);
                        _dict.push_back(make_tuple(_dictionary_key, tmp));
                        DictionaryPtr lastWordPtr = &*_dict.rbegin();
                        // Now that we know it, update the dictIdx
                        lastWordPtr->_t2.begin()->_u._variable._dictPtr = lastWordPtr;
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

int Node::_memory[MEMORY_SIZE] = {0};
unsigned Node::_currentMemoryOffset = 0;
RuntimePhrases _stack;
DictionaryType _dict;
