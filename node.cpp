#include "forth_types.h"

Node Node::makeLiteral(int intVal)
{
    Node tmp;
    tmp._kind = LITERAL;
    tmp._u._literal._intVal = intVal;
    return tmp;
}

Node Node::makeConstant(DictionaryPtr dictPtr) {
    Node tmp;
    tmp._kind = CONSTANT;
    tmp._u._constant._dictPtr = dictPtr;
    return tmp;
}

Node Node::makeVariable(DictionaryPtr dictPtr, int intVal) {
    Node tmp;
    tmp._kind = VARIABLE;
    tmp._u._variable._dictPtr = dictPtr;

    tmp._u._variable._memoryPtr = &_memory[_currentMemoryOffset];
    _currentMemoryOffset++;
    if (_currentMemoryOffset >= MEMORY_SIZE)
        error("Out of memory...");
    // later, for allot
    //tmp._u._variable._memorySize = sizeof(int);
    *tmp._u._variable._memoryPtr = intVal;
    return tmp;
}

Node Node::makeCFunction(DictionaryPtr dictPtr, FuncPtr funcPtr) {
    Node tmp;
    tmp._kind = C_FUNC;
    tmp._u._function._dictPtr = dictPtr;
    tmp._u._function._funcPtr = funcPtr;
    return tmp;
}

Node Node::makeWord(DictionaryPtr dictPtr) {
    Node tmp;
    tmp._kind = WORD;
    tmp._u._word._dictPtr = dictPtr;
    return tmp;
}

void Node::id() {
    switch(_kind) {
    case LITERAL:
        dprintf("Literal = %d\n", _u._literal._intVal);
        break;
    case CONSTANT:
        dprintf("Constant = %d\n", _u._constant._intVal);
        break;
    case VARIABLE:
        dprintf("Variable = 0x%x\n", _u._variable._memoryPtr);
        break;
    case C_FUNC:
        dprintf("Function\n");
        break;
    case WORD:
        dprintf("Word\n");
        break;
    default:
        DASSERT(false);
    }
}

void Node::dots() {
    switch(_kind) {
    case LITERAL:
        dprintf("%d", _u._literal._intVal);
        break;
    case CONSTANT:
        dprintf("%s", (char *) _u._constant._dictPtr->_t1);
        break;
    case VARIABLE:
        dprintf("%s", (char *) _u._variable._dictPtr->_t1);
        break;
    case C_FUNC:
        dprintf("%s", (char *) _u._function._dictPtr->_t1);
        break;
    case WORD:
        dprintf("%s", (char *) _u._word._dictPtr->_t1);
        break;
    default:
        DASSERT(false);
    }
}

SuccessOrFailure Node::execute()
{
    SuccessOrFailure ret = SUCCESS;
    switch(_kind) {
    case LITERAL:
        _stack.push_back(NrOrIdx::makeNr(_u._literal._intVal));
        break;
    case VARIABLE:
        _stack.push_back(NrOrIdx::makePtr(_u._variable._dictPtr));
        break;
    case CONSTANT:
        _stack.push_back(NrOrIdx::makeNr(_u._constant._intVal));
        break;
    case C_FUNC:
        ret = _u._function._funcPtr();
        break;
    case WORD:
        for(auto& node: _u._word._dictPtr->_t2) {
            if (!node.execute())
                return FAILURE;
        }
    }
    return ret;
}

// Type-specific helpers

int Node::getLiteralValue()
{
    switch(_kind) {
    case LITERAL:
        return _u._literal._intVal;
    case CONSTANT:
        return _u._constant._intVal;
    default:
        DASSERT(false);
        return -1; // to appease warning
    }
}

void Node::setConstantValue(int intVal)
{
    DASSERT(_kind == CONSTANT);
    _u._constant._intVal = intVal;
}

void Node::setVariableValue(int intVal)
{
    DASSERT(_kind == VARIABLE);
    *_u._variable._memoryPtr = intVal;
}

int Node::getVariableValue()
{
    DASSERT(_kind == VARIABLE);
    return *_u._variable._memoryPtr;
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
