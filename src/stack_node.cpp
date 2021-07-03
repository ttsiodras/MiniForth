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
    tmp._kind = PTR;
    tmp._u.dictPtr = dictPtr;
    return tmp;
}

// The action taken when ".S" is issued.
void StackNode::dots()
{
    switch(_kind) {
    case LIT:
        dprintf("%d ", _u.intVal);
        break;
    case PTR:
        // Get the name from the first field of the tuple
        // in the dictionary entry.
        dprintf("%s ", (char *) _u.dictPtr->_t1);
        break;
    default:
        DASSERT(false, "Unknown kind in StackNode::dots");
    }
}
