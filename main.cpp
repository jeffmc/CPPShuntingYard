#include <iostream>
#include <cstring>

char* concat(int start, int argc, const char* argv[]) {
    size_t sum = 0;
    for (int i=start;i<argc;++i) {
        const size_t len = strlen(argv[i]);
        sum += len;
        if (len > 0) ++sum;
    }
    if (sum < 1) return new char('\0');
    
    char* newStr = new char[sum+3];
    *newStr = '\0';

    for (int i=start;i<argc;++i) {
        const size_t len = strlen(argv[i]);
        if (len > 0) {
            strcat(newStr, argv[i]);
            strcat(newStr, " ");
        }
    }

    return newStr;
}

enum class MOperator { ADD, SUB, MULT, DIV, PWR };
char opChar(MOperator op) {
    switch (op) {
        case MOperator::ADD: return '+';
        case MOperator::SUB: return '-';
        case MOperator::MULT: return '*';
        case MOperator::DIV: return '/';
        case MOperator::PWR: return '^';
        default: return '~';
    }
}
class MToken {
    union { int v_int; MOperator v_operator; };
    enum class Type { N, INTEGER, OPERATOR } type; // N is NULL
public:
    MToken() : type(Type::N) { }
    MToken(int v) : type(Type::INTEGER), v_int(v) { }
    MToken(MOperator o) : type(Type::OPERATOR), v_operator(o) { }

    int value_int() { return v_int; }
    MOperator value_operator() { return v_operator; }
    void print() {
        switch (type) {
            case Type::N: return;
            case Type::INTEGER: printf("%i", v_int); return;
            case Type::OPERATOR: printf("%c", opChar(v_operator)); return;
        }
    }
};

// return count
size_t tokenize(const char* str, MToken* arr) {
    size_t sz;
    size_t len = strlen(str);
    for (int i=0;i<len;++i) {
        int c = (int) str[i];
        if (iscntrl(c) || isblank(c)) continue;
        if (isdigit(c)) {
            new (arr + (sz++)) MToken(c);
        }
        else if (c == '+') {
            new (arr + (sz++)) MToken(MOperator::ADD);
        }
        else if (c == '-') {
            new (arr + (sz++)) MToken(MOperator::SUB);
        }
        else if (c == '*') {
            new (arr + (sz++)) MToken(MOperator::MULT);
        }
        else if (c == '/' || c == '\\') {
            new (arr + (sz++)) MToken(MOperator::DIV);
        }
        else if (c == '*') {
            new (arr + (sz++)) MToken(MOperator::PWR);
        }
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
        printf("\n  ");
    }

    return 0;
}