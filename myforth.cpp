#include <iostream>
#include <string>
#include <cctype>
#include <map>
#include <list>
#include <stack>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <tuple>

using namespace std;

///////////////////////////////////////////////////
// Helpers for error messages and error propagation

enum SuccessOrFailure {
    FAILURE,
    SUCCESS
};

SuccessOrFailure error(const string msg) {
    cerr << "[x] " << msg << endl;
    return FAILURE;
}

template <class T>
SuccessOrFailure error(const string msg, const T& data) {
    cerr << "[x] " << msg << " " << data << endl;
    return FAILURE;
}

////////////
// The Beast

class Forth {

    const static int MEMORY_SIZE = 128;

    // Class inner-types

    typedef tuple<SuccessOrFailure, int> EvalResult;

    // The kinds of Forth constructs we support
    enum NodeType {
        LITERAL,
        CONSTANT,
        VARIABLE,
        C_FUNC,
        WORD
    };

    class Node {
    public:
        // Both interface and default implementation:
        // Executing basic nodes just means putting them
        // on the stack.
        virtual SuccessOrFailure execute() {
            _stack.push_back(this);
            return SUCCESS;
        }
        virtual void id() = 0;   // Debugging helper
        virtual void name() = 0; // Debugging helper
        virtual NodeType kind() = 0; // run-time sneak-peeks
        virtual ~Node() {};      // Mandatory for virtual classes
    };

    // The basic stuff - naked numbers.
    class LiteralNode : public Node {
        int _intVal;
    public:
        LiteralNode(int intVal):_intVal(intVal) { }
        void id()                               { cerr << "LiteralNode = " << _intVal; }
        void name()                             { cerr << _intVal; }
        NodeType kind()                         { return LITERAL; }
        int getLiteralValue()                   { return _intVal; }
    };

    // Named constants
    class ConstantNode : public Node {
        int _intVal;
        string _constantName;
    public:
        ConstantNode(const char *constantName):
            _constantName(constantName) {}
        void setConstantValue(int intVal) { _intVal = intVal; }
        SuccessOrFailure execute() {
            _stack.push_back(new LiteralNode(_intVal));
            return SUCCESS;
        }
        void id()       { cerr << "ConstantNode(" << _constantName << ") = " <<_intVal; }
        void name()     { cerr << _constantName; }
        NodeType kind() { return CONSTANT; }
    };

    // Memory access.
    class VariableNode : public Node {
        string _varName;
        int *_memoryPtr;
        // later, for allot
        //int _memorySize;
    public:
        VariableNode(const char *varName): _varName(varName) {
            _memoryPtr = &_memory[_currentMemoryOffset];
            // later, for allot
            //_memorySize = sizeof(int);
            _currentMemoryOffset++;
            if (_currentMemoryOffset >= MEMORY_SIZE)
                error("Out of memory...");
        }

        void id()                      { cerr << "VariableNode(" << _varName << ") = " << _memoryPtr; }
        void name()                    { cerr << _varName; }
        NodeType kind()                { return VARIABLE; }

        void setVariableValue(int val) { *_memoryPtr = val; }
        int getVariableValue()         { return *_memoryPtr; }

        // later, for allot
        // void setVariableSize(int newSize)
        // {
        //     _currentMemoryOffset -= sizeof(int);
        //     _memoryOffset = _currentMemoryOffset;
        //     _memorySize = newSize;
        //     _currentMemoryOffset += _memorySize;
        // }
    };

    typedef SuccessOrFailure (*FuncPtr)();

    class C_FuncNode : public Node {
        string _funcName;
        FuncPtr _funcPtr;
    public:
        C_FuncNode(const char *funcName, FuncPtr funcPtr) {
            _funcName = funcName;
            _funcPtr = funcPtr;
        }
        SuccessOrFailure execute() { return _funcPtr(); }
        void id()                  { cerr << "C_FuncNode " << _funcName; }
        void name()                { cerr << _funcName; }
        NodeType kind()            { return C_FUNC; }
    };

    class WordNode : public Node {
        string _wordName;
        list<Node*> _nodes;
    public:
        WordNode(const string& wordName):
            _wordName(wordName) {}

        void addWord(Node *pNode) { _nodes.push_back(pNode); }
        void setWordName(const string& name) { _wordName = name; }

        SuccessOrFailure execute() {
            for(auto node: _nodes) {
                if (!node->execute())
                    return FAILURE;
            }
            return SUCCESS;
        }

        void id() {
            cerr << "WordNode(" << _wordName << ":\n\t";
            for(auto node: _nodes) { node->id(); cerr << "\n\t"; }
        }
        void name()     { cerr << _wordName; }
        NodeType kind() { return WORD; }
    };

    // Class-globals

    // The memory used to store things like strings/arrays
    static int      _memory[MEMORY_SIZE];
    static unsigned _currentMemoryOffset;

    // The run-time stack
    static list<Node*> _stack;

    // The dictionary
    typedef        map<string, Node*> DictionaryType;
    DictionaryType _dict;

    // The currently being populated dictionary key
    string _dictionary_key;

    // ...which is only different from "" when we are compiling:
    bool     _compiling = false;
    WordNode _wordBeingDefined;

    // Interpreter state-machine-related variables
    bool definingConstant = false;
    bool definingVariable = false;

    static EvalResult evaluate_stack_top(const char *errorMessage)
    {
        if(_stack.empty()) {
            if(errorMessage)
                error(errorMessage);
            return make_tuple(FAILURE, -1);
        }
        Node *pNode = *_stack.rbegin(); 
        _stack.pop_back();
        // cerr << "Evaluating: (" << pNode << ") ";
        // pNode->id();
        // cerr << "\n";
        pNode->execute();
        pNode = *_stack.rbegin(); 
        if (pNode->kind() != LITERAL) {
            error("[x] Evaluation did not create a number...");
            return make_tuple(FAILURE, -1);
        } else {
            LiteralNode *pLiteralNode = dynamic_cast<LiteralNode*>(pNode);
            _stack.pop_back();
            return make_tuple(SUCCESS, pLiteralNode->getLiteralValue());
        }
    }

    static bool commonArithmetic(int& v1, int& v2, const char *msg)
    {
        bool success;
        tie(success, v1) = evaluate_stack_top(msg);
        if (!success)
            return FAILURE;
        tie(success, v2) = evaluate_stack_top(msg);
        if (!success)
            return FAILURE;
        return SUCCESS;
    }

    static SuccessOrFailure add(void)
    {
        int v1, v2;
        if (!commonArithmetic(v1, v2, "'+' needs two arguments..."))
            return FAILURE;
        _stack.push_back(new LiteralNode(v2+v1));
        return SUCCESS;
    }

    static SuccessOrFailure sub(void)
    {
        int v1, v2;
        if(!commonArithmetic(v1, v2, "'-' needs two arguments..."))
            return FAILURE;
        _stack.push_back(new LiteralNode(v2-v1));
        return SUCCESS;
    }

    static SuccessOrFailure mul(void)
    {
        int v1, v2;
        if(!commonArithmetic(v1, v2, "'*' needs two arguments..."))
            return FAILURE;
        _stack.push_back(new LiteralNode(v2*v1));
        return SUCCESS;
    }

    static SuccessOrFailure div(void)
    {
        int v1, v2;
        if(!commonArithmetic(v1, v2, "'/' needs two arguments..."))
            return FAILURE;
        if (!v2)
            return error("Division by zero...");
        _stack.push_back(new LiteralNode(v2/v1));
        return SUCCESS;
    }

    static SuccessOrFailure dot(void)
    {
        SuccessOrFailure success; int v;
        tie(success, v) = evaluate_stack_top("Nothing on the stack...");
        if (success) {
            cout << v;
        }
        return success;
    }

    static SuccessOrFailure dots(void)
    {
        cout << "[ ";
        for(auto node: _stack) {
            node->name();
            cout << " ";
        }
        cout << "]";
        return SUCCESS;
    }

    static SuccessOrFailure at(void)
    {
        const char *errMsg = "@ needs a variable on the stack";
        if (_stack.empty())
            return error(errMsg);
        if (VARIABLE != (*_stack.rbegin())->kind())
            return error(errMsg);
        VariableNode *pVarNode = dynamic_cast<VariableNode*>(*_stack.rbegin());
        _stack.pop_back();
        _stack.push_back(new LiteralNode(pVarNode->getVariableValue()));
        return SUCCESS;
    }

    static SuccessOrFailure bang(void)
    {
        const char *errMsg = "! needs a variable and a value on the stack";
        if (_stack.empty())
            return error(errMsg);

        // First, get the variable
        if (VARIABLE != (*_stack.rbegin())->kind())
            return error(errMsg);
        VariableNode *pVarNode = dynamic_cast<VariableNode*>(*_stack.rbegin());
        _stack.pop_back();

        // Then, compute the value
        SuccessOrFailure success; int v;
        tie(success, v) = evaluate_stack_top("Failed to evaluate value for !...");
        if (success) {
            pVarNode->setVariableValue(v);
            return SUCCESS;
        }
        return FAILURE;
    }

public:
    Forth():
        _dictionary_key(""),
        _compiling(false),
        _wordBeingDefined("")
    {
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
            { ".s", &Forth::dots }
        };

        for(auto cmd: c_ops)
            _dict[cmd.name] = new C_FuncNode(cmd.name, cmd.funcPtr);
    }

    tuple<bool,Node*> compile_word(const string& word)
    {
        if (all_of(word.begin(), word.end(),
                   [](char c) { return isdigit(c); }))
            return make_tuple(true, new LiteralNode(atoi(word.c_str())));
        DictionaryType::iterator it = _dict.find(word);
        if (it == _dict.end()) {
            error("Unknown word:", word);
            return make_tuple(false, (Node*)NULL);
        }
        return make_tuple(true, it->second);
    }

    auto interpret(const string& word)
    {
        if (word == "variable") {
            // We expect vars/constants to have a default initialization value
            if (_stack.empty())
                return error("You forgot to initialise the variable...");
            definingVariable = true;
        } else if (word == "constant") {
            // We expect vars/constants to have a default initialization value
            if (_stack.empty())
                return error("You forgot to initialise the constant...");
            definingConstant = true;
        } else if (all_of(word.begin(), word.end(), [](char c) { return isdigit(c); })) {
            _stack.push_back(new LiteralNode(atoi(word.c_str())));
        } else {
            // Must be in the dictionary
            DictionaryType::iterator it = _dict.find(word);
            if (it == _dict.end())
                return error("No such symbol found: ", word);
            return it->second->execute();
        }
        return SUCCESS;
    }

    auto parse_line(const char *begin, const char *end)
    {
        uint32_t words=0;
        const char *p=begin;
        
        while(1) {
            // Skip spaces
            while(p<end && isspace(*p))
                p++;

            // No more data?
            if (p == end)
                break;

            // Read space-delimited word
            string word = "";
            while(p<end && !isspace(*p))
                word += *p++;

            // Parse word
            if (word == ":") {
                _compiling = true;
                _wordBeingDefined = WordNode("");
            } else if (word == ";") {
                if (_compiling) {
                    _compiling = false;
                    _dict[_dictionary_key] = new WordNode(_wordBeingDefined);
                    _dictionary_key = "";
                    if (definingVariable)
                        return error("[x] You didn't finish defining the variable...");
                    if (definingConstant)
                        return error("[x] You didn't finish defining the constant...");
                } else
                    return error("[x] Not in compiling mode...");
            } else {
                if (_compiling) {
                    if (_dictionary_key == "") {
                        _dictionary_key = word;
                        _wordBeingDefined.setWordName(word);
                    } else {
                        auto ret = compile_word(word);
                        if (!get<0>(ret))
                            return error("[x] Failed to parse word:", word);
                        _wordBeingDefined.addWord(get<1>(ret));
                    }
                } else {
                    if (definingConstant) {
                        bool success; int v;
                        tie(success, v) = evaluate_stack_top(
                            "[x] Failure computing constant...");
                        if (success) {
                            auto c = new ConstantNode(word.c_str());
                            c->setConstantValue(v);
                            _dict[word] = c;
                        }
                        definingConstant = false;
                    } else if (definingVariable) {
                        bool success; int v;
                        tie(success, v) = evaluate_stack_top(
                            "[x] Failure computing variable initial value...");
                        if (success) {
                            auto vNode = new VariableNode(word.c_str());
                            vNode->setVariableValue(v);
                            _dict[word] = vNode;
                        }
                        definingVariable = false;
                    } else {
                        if (!interpret(word))
                            break;
                    }
                }
            }
            words++;
        }
        return SUCCESS;
    }
};

int Forth::_memory[MEMORY_SIZE] = {0};
unsigned Forth::_currentMemoryOffset = 0;
list<Forth::Node*> Forth::_stack;

int main()
{
    Forth miniforth;
    string line;
    while(getline(cin, line)) {
        if (miniforth.parse_line(line.c_str(), line.c_str() + line.length()))
            cout << " OK" << endl;
    }
    cout << "All done.\n";
}
