#include "stack_node.h"
#include "helpers.h"
#include "dassert.h"
#include "miniforth.h"

StackNode StackNode::makeNr(int intVal)
{
    StackNode tmp;
    tmp._kind = LIT;
    tmp._u.intVal = intVal;
    return tmp;
}

StackNode StackNode::makePtr(DictionaryPtr dictPtr)
{
    StackNode tmp;
    tmp._kind = IDX;
    tmp._u.dictPtr = dictPtr;
    return tmp;
}

int StackNode::getLiteralValue()
{
    DASSERT(isLiteral(), "StackNode::getLiteralValue");
    return _u.intVal;
}

void StackNode::dots()
{
    switch(_kind) {
    case LIT:
        dprintf("%d ", _u.intVal);
        break;
    case IDX:
        dprintf("%s ", (char *) _u.dictPtr->_t1);
        break;
    default:
        DASSERT(false, "Unknown kind in StackNode::dots");
    }
}
