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
        (void) error(F("Out of memory..."));
    *tmp._u._variable._memoryPtr = intVal;
    return tmp;
}

CompiledNode CompiledNode::makeCFunction(const char *addrOfNameOfFunctionInFlash, FuncPtr funcPtr) {
    CompiledNode tmp;
    tmp._kind = C_FUNC;
    tmp._u._function._addrOfNameOfFunctionInFlash = addrOfNameOfFunctionInFlash;
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
        if(!run_full_phrase(_u._word._dictPtr->getCompiledNodes()))
            return it;
        break;
    case UNKNOWN:
        break;
    }
    return ret;
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
    // The heart of the engine...
    //
    // Begin at the first CompiledNode in our word
    auto it = compiled_nodes.begin();
    while(it != compiled_nodes.end()) {
        // First, deal with the IF execution stack.
        // To support nested IF/ELSE/THEN, we need an IF stack
        // (stored in Forth::_ifStates). As we can our word's 
        // CompiledNode s, an IF will push on this; a THEN will pop.
        // If the top-most entry tells us that the pushed value
        // was true, then we allow execution of the CompiledNode s
        // within the IF body; otherwise we allow execution of
        // CompiledNode s within the ELSE body.
        // Whether we are inside an IF or an ELSE body is coded
        // in the class global (static) inside_IF_body, updated
        // as we move along (see the 'execute' methods for IF/ELSE/THEN).
        bool itIsTHEN = it->_kind == CompiledNode::C_FUNC && !strcasecmp_P(it->getWordName(), (PGM_P) F("THEN"));
        bool itIsELSE = it->_kind == CompiledNode::C_FUNC && !strcasecmp_P(it->getWordName(), (PGM_P) F("ELSE"));
        // Is our IF stack not empty? Then we are in code following an IF...
        // Apply IF/ELSE logic to see if we should execute the CompiledNode.
        // But *always* evaluate the ELSE/THENs, to update IF stack/state.
        if (!Forth::_ifStates.empty() && !itIsTHEN && !itIsELSE) {
            if ((IfState::inside_IF_body && !Forth::_ifStates.begin()->wasTrue()) ||
                (!IfState::inside_IF_body && Forth::_ifStates.begin()->wasTrue()))
            {
                ++it;
                continue;
            }

        }
        auto ret = it->execute(it);
        // A CompiledNode may choose to tell us it failed to execute;
        // e.g. a '+' that didn't find two elements on the stack.
        if (!ret)
            return FAILURE;
        // A CompiledNode may choose to tell us to change the "program counter"
        // (i.e. the iterator we are using to run through the words)
        if (it != ret.value())
            // Jump! E.g. in a DO ... LOOP. the LOOP returns the iterator to: DO
            it = ret.value();
        else
            ++it;
    }
    return SUCCESS;
}
