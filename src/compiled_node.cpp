#include "miniforth.h"
#include "compiled_node.h"
#include "helpers.h"
#include "dassert.h"

CompiledNode::CompiledNode() {}

CompiledNode CompiledNode::makeLiteral(int intVal)
{
    CompiledNode tmp;
    tmp._kind = LITERAL;
    tmp._u._literal._intVal = intVal;
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
        error("Out of memory...");
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

void CompiledNode::id() {
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
        dprintf("%s", "Function\n");
        break;
    case WORD:
        dprintf("%s", "Word\n");
        break;
    default:
        DASSERT(false, "Reached default in CompiledNode::id");
    }
}

void CompiledNode::dots() {
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
        DASSERT(false, "Reached default in CompiledNode::dots");
    }
}

SuccessOrFailure CompiledNode::execute()
{
    SuccessOrFailure ret = SUCCESS;
    switch(_kind) {
    case LITERAL:
        Forth::_stack.push_back(StackNode::makeNr(_u._literal._intVal));
        break;
    case VARIABLE:
        Forth::_stack.push_back(StackNode::makePtr(_u._variable._dictPtr));
        break;
    case CONSTANT:
        Forth::_stack.push_back(StackNode::makeNr(_u._constant._intVal));
        break;
    case C_FUNC:
        ret = _u._function._funcPtr();
        break;
    case WORD:
        auto& nodes = _u._word._dictPtr->_t2;
        for(int j=0; j<nodes._currentSize; j++) {
            CompiledNode& node = nodes[j];
            if (!node.execute())
                return FAILURE;
        }
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

// later, for allot
// void setVariableSize(int newSize)
// {
//     dassert(_kind == VARIABLE);
//     _currentMemoryOffset -= sizeof(int);
//     _memoryOffset = _currentMemoryOffset;
//     _memorySize = newSize;
//     _currentMemoryOffset += _memorySize;
// }
