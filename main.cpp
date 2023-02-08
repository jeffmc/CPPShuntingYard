#include <iostream>
#include <cstring>

char* concat(int start, int argc, const char* argv[]) {
    size_t sz = 0;
    for (int i=start;i<argc;++i) {
        const size_t len = strlen(argv[i]);
        sz += len + 1;
    }
    if (sz < 1) return new char('\0');
    
    char* newStr = new char[sz+3];
    *newStr = '\0';

    for (int i=start;i<argc;++i) {
        const size_t len = strlen(argv[i]);
        if (len > 0) {
            strcat(newStr, argv[i]);
            if (i < argc-1) strcat(newStr, " ");
        }
    }

    return newStr;
}

enum class MOperator { ADD, SUB, MULT, DIV, PWR, MOD };
struct MOpPair {MOperator op; char c; };
const MOpPair opPairs[] = {
    {MOperator::ADD, '+'},
    {MOperator::SUB, '-'},
    {MOperator::MULT, '*'},
    {MOperator::DIV, '/'},
    {MOperator::PWR, '^'},
    {MOperator::MOD, '%'},
};
const size_t opCount = sizeof(opPairs) / sizeof(opPairs[0]);

char opChar(MOperator op) {
    for (int i=0;i<opCount;++i) if (op == opPairs[i].op) return opPairs[i].c;
    return '\0';
}
class MToken {
    union { int v_int; MOperator v_operator; bool v_open; };
    enum class Type { N, INTEGER, OPERATOR, GROUP } type; // N is NULL
public:
    MToken() : type(Type::N) { } // null/empty/whitespace
    MToken(int v) : type(Type::INTEGER), v_int(v) { } // numeral constants
    MToken(MOperator o) : type(Type::OPERATOR), v_operator(o) { } // operators
    MToken(bool o) : type(Type::GROUP), v_open(o) { } // group

    int value_int() { return v_int; }
    MOperator value_operator() { return v_operator; }
    bool value_group_opener() { return v_open; }

    void print() {
        switch (type) {
            case Type::N: return;
            case Type::INTEGER: printf("%i", v_int); return;
            case Type::OPERATOR: printf("%c", opChar(v_operator)); return;
            case Type::GROUP: printf("%c", v_open ? '(' : ')'); return;
        }
    }
};

// return count
size_t tokenize(const char* str, MToken* arr) {
    size_t sz = 0;
    size_t len = strlen(str);
    for (int i=0;i<len;++i) {
        char ch = str[i];
        int c = (int) ch;
        printf("%i, %c\n", c, ch);
        if (iscntrl(c) || isblank(c)) continue;
        if (isdigit(c)) {
            arr[sz++] = MToken(c - 48);
        }
        else if (c == '+') { arr[sz++] = MToken(MOperator::ADD); } // TODO: Find out how to use placement new here!
        else if (c == '-') { arr[sz++] = MToken(MOperator::SUB); }
        else if (c == '*') { arr[sz++] = MToken(MOperator::MULT); }
        else if (c == '/' || c == '\\') { arr[sz++] = MToken(MOperator::DIV); }
        else if (c == '^') { arr[sz++] = MToken(MOperator::PWR); }
        else if (c == '(') { arr[sz++] = MToken(true); }
        else if (c == ')') { arr[sz++] = MToken(false); }
    }
    return sz;
}

int main(int argc, const char* argv[]) {
    const char* comb = concat(1,argc,argv);
    printf("Raw: \"%s\"\n", comb);

    MToken* tkns = new MToken[strlen(comb)];
    size_t tknCt = tokenize(comb, tkns);

    printf("Tokenized:\n  ");
    for (int i=0;i<tknCt;++i) {
        tkns[i].print();
        printf(" ");
    }
    printf("\n");

    return 0;
}