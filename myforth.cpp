#include <iostream>
#include <string>
#include <cctype>
#include <map>
#include <list>
#include <stack>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <tuple>

using namespace std;

#define MAX_NAME_LENGTH 32

///////////////////////////////////////////////////
// Helpers for error messages and error propagation

#define SAFE_STRCPY(dest, src) ::strncpy(&dest[0], src, sizeof(dest))

enum SuccessOrFailure {
    FAILURE,
    SUCCESS
};

SuccessOrFailure error(const char *msg) {
    cerr << "[x] " << msg << endl;
    return FAILURE;
}

template <class T>
SuccessOrFailure error(const char *msg, const T& data) {
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

    // Type used for C_FUNC callbacks
    typedef SuccessOrFailure (*FuncPtr)();

    struct Node {
        NodeType _kind;
        union UnionData {
            UnionData() {}
            struct {
                int _intVal;
            } _literal;
            struct {
                int _intVal;
                char _constantName[MAX_NAME_LENGTH];
            } _constant;
            struct {
                int *_memoryPtr;
                char _varName[MAX_NAME_LENGTH];
                // later, for allot
                //int _memorySize;
            } _variable;
            struct {
                char _funcName[MAX_NAME_LENGTH];
                FuncPtr _funcPtr;
            } _function;
            struct {
                char _wordName[MAX_NAME_LENGTH];
            } _word;
        } _u;

        Node() {}

        static Node makeLiteral(int intVal) {
            Node tmp;
            tmp._kind = LITERAL;
            tmp._u._literal._intVal = intVal;
            return tmp;
        }

        static Node makeConstant(const char *constantName) {
            Node tmp;
            tmp._kind = CONSTANT;
            SAFE_STRCPY(tmp._u._constant._constantName, constantName);
            return tmp;
        }

        static Node makeVariable(const char *varName, int intVal) {
            Node tmp;
            tmp._kind = VARIABLE;
            SAFE_STRCPY(tmp._u._variable._varName, varName);

            tmp._u._variable._memoryPtr = &_memory[_currentMemoryOffset];
            _currentMemoryOffset++;
            if (_currentMemoryOffset >= MEMORY_SIZE)
                error("Out of memory...");
            // later, for allot
            //tmp._u._variable._memorySize = sizeof(int);
            *tmp._u._variable._memoryPtr = intVal;
            return tmp;
        }

        static Node makeCFunction(const char *funcName, FuncPtr funcPtr) {
            Node tmp;
            tmp._kind = C_FUNC;
            SAFE_STRCPY(tmp._u._function._funcName, funcName);
            tmp._u._function._funcPtr = funcPtr;
            return tmp;
        }

        static Node makeWord(const char *wordName) {
            Node tmp;
            tmp._kind = WORD;
            SAFE_STRCPY(tmp._u._word._wordName, wordName);
            return tmp;
        }

        void id() {
            switch(_kind) {
            case LITERAL:
                cerr << "Literal = " << _u._literal._intVal;
                break;
            case CONSTANT:
                cerr << "Constant " << _u._constant._constantName;
                cerr << " = " << _u._constant._intVal;
                break;
            case VARIABLE:
                cerr << "Variable " << _u._variable._varName;
                cerr << " = " << _u._variable._memoryPtr;
                break;
            case C_FUNC:
                cerr << "Function " << _u._function._funcName;
                break;
            case WORD:
                cerr << "Word " << _u._word._wordName;
                break;
            default:
                assert(false);
            }
        }

        void dots() {
            switch(_kind) {
            case LITERAL:
                cerr << _u._literal._intVal;
                break;
            case CONSTANT:
                cerr << _u._constant._constantName;
                break;
            case VARIABLE:
                cerr << _u._variable._varName;
                break;
            case C_FUNC:
                cerr << _u._function._funcName;
                break;
            case WORD:
                cerr << _u._word._wordName;
                break;
            default:
                assert(false);
            }
        }

        SuccessOrFailure execute() {
            switch(_kind) {
            case LITERAL:
            case VARIABLE:
                _stack.push_back(*this);
                break;
            case CONSTANT:
                _stack.push_back(Node::makeLiteral(_u._constant._intVal));
                break;
            case C_FUNC:
                _u._function._funcPtr();
                break;
            case WORD:
                DictionaryType::iterator it = _dict.find(_u._word._wordName);
                if (it == _dict.end())
                    return FAILURE;
                for(Node& node: it->second) {
                    if (!node.execute())
                        return FAILURE;
                }
            }
            return SUCCESS;
        }

        // Type-specific helpers

        int getLiteralValue() {
            switch(_kind) {
            case LITERAL:
                return _u._literal._intVal;
            case CONSTANT:
                return _u._constant._intVal;
            default:
                assert(false);
            }
        }

        void setConstantValue(int intVal) {
            assert(_kind == CONSTANT);
            _u._constant._intVal = intVal;
        }

        void setVariableValue(int intVal) {
            assert(_kind == VARIABLE);
            *_u._variable._memoryPtr = intVal;
        }

        int getVariableValue() {
            assert(_kind == VARIABLE);
            return *_u._variable._memoryPtr;
        }

        // later, for allot
        // void setVariableSize(int newSize)
        // {
        //     assert(_kind == VARIABLE);
        //     _currentMemoryOffset -= sizeof(int);
        //     _memoryOffset = _currentMemoryOffset;
        //     _memorySize = newSize;
        //     _currentMemoryOffset += _memorySize;
        // }
    };

    // Class-globals

    // The memory used to store things like strings/arrays
    static int      _memory[MEMORY_SIZE];
    static unsigned _currentMemoryOffset;

    // The run-time stack
    static list<Node> _stack;

    // The dictionary
    typedef map<string, list<Node>> DictionaryType;
    static DictionaryType _dict;

    // The currently being populated dictionary key
    string _dictionary_key;

    // ...which is only different from "" when we are compiling:
    bool _compiling = false;

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
        Node node = *_stack.rbegin(); 
        _stack.pop_back();
        // cerr << "Evaluating: (" << pNode << ") ";
        // pNode->id();
        // cerr << "\n";
        node.execute();
        node = *_stack.rbegin(); 
        if (node._kind != LITERAL) {
            error("Evaluation did not create a number...");
            return make_tuple(FAILURE, -1);
        } else {
            _stack.pop_back();
            return make_tuple(SUCCESS, node.getLiteralValue());
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
        _stack.push_back(Node::makeLiteral(v2+v1));
        return SUCCESS;
    }

    static SuccessOrFailure sub(void)
    {
        int v1, v2;
        if(!commonArithmetic(v1, v2, "'-' needs two arguments..."))
            return FAILURE;
        _stack.push_back(Node::makeLiteral(v2-v1));
        return SUCCESS;
    }

    static SuccessOrFailure mul(void)
    {
        int v1, v2;
        if(!commonArithmetic(v1, v2, "'*' needs two arguments..."))
            return FAILURE;
        _stack.push_back(Node::makeLiteral(v2*v1));
        return SUCCESS;
    }

    static SuccessOrFailure div(void)
    {
        int v1, v2;
        if(!commonArithmetic(v1, v2, "'/' needs two arguments..."))
            return FAILURE;
        if (!v2)
            return error("Division by zero...");
        _stack.push_back(Node::makeLiteral(v2/v1));
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
        for(Node& node: _stack) {
            node.dots();
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
        Node tmp = *_stack.rbegin();
        if (VARIABLE != tmp._kind)
            return error(errMsg);
        _stack.pop_back();
        _stack.push_back(Node::makeLiteral(tmp.getVariableValue()));
        return SUCCESS;
    }

    static SuccessOrFailure bang(void)
    {
        const char *errMsg = "! needs a variable and a value on the stack";
        if (_stack.empty())
            return error(errMsg);

        // First, get the variable
        Node tmp = *_stack.rbegin();
        if (VARIABLE != tmp._kind)
            return error(errMsg);
        _stack.pop_back();

        // Then, compute the value
        SuccessOrFailure success; int v;
        tie(success, v) = evaluate_stack_top("Failed to evaluate value for !...");
        if (success) {
            tmp.setVariableValue(v);
            return SUCCESS;
        }
        return FAILURE;
    }

public:
    Forth():
        _dictionary_key(""),
        _compiling(false)
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
            _dict[cmd.name].push_back(Node::makeCFunction(cmd.name, cmd.funcPtr));
    }

    tuple<SuccessOrFailure, int> isnumber(const string& word)
    {
        if (word.empty())
            return make_tuple(FAILURE, -1);
        const char *ptrStart = word.c_str();
        char *ptrEnd;
        int base = 10;
        if (word[0] == '$') { // hex numbers
            ptrStart++;
            base = 16;
        }
        long val = strtol(ptrStart, &ptrEnd, base);
        if (ptrEnd == &word[word.length()])
            return make_tuple(SUCCESS, (int)val);
        return make_tuple(FAILURE, -1);
    }

    tuple<SuccessOrFailure,Node> compile_word(const string& word)
    {
        auto numericValue = isnumber(word);
        if (get<0>(numericValue))
            return make_tuple(SUCCESS, Node::makeLiteral(get<1>(numericValue)));
        DictionaryType::iterator it = _dict.find(word);
        if (it == _dict.end()) {
            error("Unknown word:", word);
            return make_tuple(FAILURE, Node::makeLiteral(0));
        }
        return make_tuple(SUCCESS, Node::makeWord(word.c_str()));
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
        } else {
            auto numericValue = isnumber(word);
            if (get<0>(numericValue))
                _stack.push_back(Node::makeLiteral(get<1>(numericValue)));
            else {
                // Must be in the dictionary
                DictionaryType::iterator it = _dict.find(word);
                if (it == _dict.end())
                    return error("No such symbol found: ", word);
                for(Node& node: it->second) {
                    if (!node.execute())
                        return FAILURE;
                }
            }
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
            } else if (word == ";") {
                if (_compiling) {
                    _compiling = false;
                    _dictionary_key = "";
                    if (definingVariable)
                        return error("You didn't finish defining the variable...");
                    if (definingConstant)
                        return error("You didn't finish defining the constant...");
                } else
                    return error("Not in compiling mode...");
            } else {
                if (_compiling) {
                    if (_dictionary_key == "") {
                        _dictionary_key = word;
                    } else {
                        auto ret = compile_word(word);
                        if (!get<0>(ret))
                            return error("Failed to parse word:", word);
                        _dict[_dictionary_key].push_back(get<1>(ret));
                    }
                } else {
                    if (definingConstant) {
                        bool success; int v;
                        tie(success, v) = evaluate_stack_top(
                            "[x] Failure computing constant...");
                        if (success) {
                            auto c = Node::makeConstant(word.c_str());
                            c.setConstantValue(v);
                            _dict[word].clear();
                            _dict[word].push_back(c);
                        }
                        definingConstant = false;
                    } else if (definingVariable) {
                        bool success; int v;
                        tie(success, v) = evaluate_stack_top(
                            "[x] Failure computing variable initial value...");
                        if (success) {
                            auto vNode = Node::makeVariable(word.c_str(), v);
                            _dict[word].clear();
                            _dict[word].push_back(vNode);
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
list<Forth::Node> Forth::_stack;
Forth::DictionaryType Forth::_dict;

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
