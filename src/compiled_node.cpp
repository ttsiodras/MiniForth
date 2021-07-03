#include "miniforth.h"
#include "compiled_node.h"
#include "helpers.h"
#include "dassert.h"

CompiledNode::CompiledNode() {}

CompiledNode CompiledNode::makeUnknown()
{
    CompiledNode tmp;
    tmp._kind = UNKNOWN;
    return tmp;
}

CompiledNode CompiledNode::makeLiteral(int intVal)
{
    CompiledNode tmp;
    tmp._kind = LITERAL;
    tmp._u._literal._intVal = intVal;
    return tmp;
}

CompiledNode CompiledNode::makeString(const char *p)
{
    CompiledNode tmp;
    tmp._kind = STRING;
    tmp._u._string._strVal = string(p);
    return tmp;
}

CompiledNode CompiledNode::makeConstant(DictionaryPtr dictPtr) {
    CompiledNode tmp;
    tmp._kind = CONSTANT;
    tmp._u._constant._dictPtr = dictPtr;
    return tmp;
}

CompiledNode CompiledNode::makeVariable(DictionaryPtr dictPtr, int intVal) {
    CompiledNode tmp;
    tmp._kind = VARIABLE;
    tmp._u._variable._dictPtr = dictPtr;

    tmp._u._variable._memoryPtr = &_memory[_currentMemoryOffset];
    _currentMemoryOffset++;
    if (_currentMemoryOffset >= MEMORY_SIZE)
        error(F("Out of memory..."));
    // later, for allot
    //tmp._u._variable._memorySize = sizeof(int);
    *tmp._u._variable._memoryPtr = intVal;
    return tmp;
}

CompiledNode CompiledNode::makeCFunction(DictionaryPtr dictPtr, FuncPtr funcPtr) {
    CompiledNode tmp;
    tmp._kind = C_FUNC;
    tmp._u._function._dictPtr = dictPtr;
    tmp._u._function._funcPtr = funcPtr;
    return tmp;
}

CompiledNode CompiledNode::makeWord(DictionaryPtr dictPtr) {
    CompiledNode tmp;
    tmp._kind = WORD;
    tmp._u._word._dictPtr = dictPtr;
    return tmp;
}

void CompiledNode::dots() {
    switch(_kind) {
    case LITERAL:
        dprintf("%d", _u._literal._intVal);
        break;
    case STRING:
        dprintf("%s", _u._string._strVal.c_str());
        break;
    case CONSTANT:
    case VARIABLE:
    case C_FUNC:
    case WORD:
        dprintf("%s", (char *) getWordName());
        break;
    case UNKNOWN:
        DASSERT(false, "UNKNOWN not expected in CompiledNode::dots");
        break;
    }
}

CompiledNode::ExecuteResult CompiledNode::execute(CompiledNodes::iterator it)
{
    auto ret = Optional<CompiledNodes::iterator>(it);
    switch(_kind) {
    case LITERAL:
        Forth::_stack.push_back(StackNode::makeNr(_u._literal._intVal));
        break;
    case STRING:
        dprintf(" %s", _u._string._strVal.c_str());
        break;
    case VARIABLE:
        Forth::_stack.push_back(StackNode::makePtr(_u._variable._dictPtr));
        break;
    case CONSTANT:
        Forth::_stack.push_back(StackNode::makeNr(_u._constant._intVal));
        break;
    case C_FUNC:
        ret = _u._function._funcPtr(it);
        break;
    case WORD:
        if(!run_full_phrase(_u._word._dictPtr->_t2))
            return it;
        break;
    case UNKNOWN:
        break;
    }
    return ret;
}

// Type-specific helpers

int CompiledNode::getLiteralValue()
{
    switch(_kind) {
    case LITERAL:
        return _u._literal._intVal;
    case CONSTANT:
        return _u._constant._intVal;
    default:
        DASSERT(false, "Reached default in CompiledNode::getLiteralValue");
        return -1; // to appease warning
    }
}

void CompiledNode::setConstantValue(int intVal)
{
    DASSERT(_kind == CONSTANT, "setConstantValue called on non-constant");
    _u._constant._intVal = intVal;
}

void CompiledNode::setVariableValue(int intVal)
{
    DASSERT(_kind == VARIABLE, "setVariableValue called on non-variable");
    *_u._variable._memoryPtr = intVal;
}

int CompiledNode::getVariableValue()
{
    DASSERT(_kind == VARIABLE, "getVariableValue called on non-variable");
    return *_u._variable._memoryPtr;
}

SuccessOrFailure CompiledNode::run_full_phrase(CompiledNodes& compiled_nodes)
{
    auto it = compiled_nodes.begin();
    while(it != compiled_nodes.end()) {
        if (!Forth::_ifStates.empty() && !(it->_kind == CompiledNode::C_FUNC &&
                                           it->getWordName() != "THEN"))
        {
            // but always evaluate the THENs, because they drain the IF stack
            if ((IfState::inside_IF_body && !Forth::_ifStates.begin()->wasTrue()) ||
                (!IfState::inside_IF_body && Forth::_ifStates.begin()->wasTrue()))
            {
                ++it;
                continue;
            }

        }
        auto ret = it->execute(it);
        if (!ret._t1)
            return FAILURE;
        if (it != ret._t2)
            it = ret._t2;
        else
            ++it;
    }
    return SUCCESS;
}

// later, for allot
// void setVariableSize(int newSize)
// {
//     dassert(_kind == VARIABLE);
//     _currentMemoryOffset -= sizeof(int);
//     _memoryOffset = _currentMemoryOffset;
//     _memorySize = newSize;
//     _currentMemoryOffset += _memorySize;
// }
