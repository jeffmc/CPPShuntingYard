#include <iostream>
#include <cstring>

#include "types.h"

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

    void int_digit_append(int num) {
        if (!is_int()) return;
        if (num < 0) num *= -1;
        v_int *= 10;
        v_int += num;
    }

    bool is_null() const { return type == Type::N; }
    bool is_int() const { return type == Type::INTEGER; }
    bool is_operator() const { return type == Type::OPERATOR; }
    bool is_group() const { return type == Type::GROUP; }

    int value_int() { return v_int; }
    MOperator value_operator() { return v_operator; }
    bool value_group_opener() { return v_open; }

    void print() const {
        switch (type) {
            case Type::N: printf("NULLTKN"); return;
            case Type::INTEGER: printf("%i", v_int); return;
            case Type::OPERATOR: printf("%c", opChar(v_operator)); return;
            case Type::GROUP: printf("%c", v_open ? '(' : ')'); return;
        }
    }
};

// return count
size_t tokenize(const char* str, Queue<MToken> &q) {
    size_t sz = 0;
    size_t len = strlen(str);
    MToken* lt = nullptr; // last token
    for (int i=0;i<len;++i) {
        char ch = str[i];
        int c = (int) ch;
        // printf("%i, %c\n", c, ch);
        if (iscntrl(c) || isblank(c)) continue;
        MToken* t;
        if (isdigit(c)) { 
            const int digit = c - 48;
            if (lt && lt->is_int()) {
                lt->int_digit_append(digit);
                t = lt; // Append to previous
            } else {
                t = new MToken(digit);
            }
        }
        else if (c == '+') { t = new MToken(MOperator::ADD); } // TODO: Find out how to use placement new here!
        else if (c == '-') { t = new MToken(MOperator::SUB); }
        else if (c == '*') { t = new MToken(MOperator::MULT); }
        else if (c == '/' || c == '\\') { t = new MToken(MOperator::DIV); }
        else if (c == '^') { t = new MToken(MOperator::PWR); }
        else if (c == '(') { t = new MToken(true); }
        else if (c == ')') { t = new MToken(false); }
        else { t = new MToken(); }

        if (t != lt) q.enqueue(t); // As long as new token isn't simply an appendage to previous, enqueue it.
        lt = t;
    }
    return sz;
}

void printQueue(const Queue<MToken> &q) {
    printf("Size: %i, (FRONT) ", q.size());
    Node<MToken>* head = q.head;
    while (head) {
        head->data->print();
        printf(" ");
        head = head->next;
    }
    printf("(BACK)\n");
}

void printStack(const Stack<MToken> &s) {
    printf("Size: %i, (TOP) ", s.size());
    Node<MToken>* head = s.head;
    while (head) {
        head->data->print();
        printf(" ");
        head = head->next;
    }
    printf("(BOTTOM)\n");
}

int main(int argc, const char* argv[]) {
    const char* comb = concat(1,argc,argv);
    printf("Raw: \"%s\"\n", comb);

    Queue<MToken> queue{};
    size_t tknCt = tokenize(comb, queue);

    printQueue(queue);

    Stack<MToken> stack{};

    printf("Tokenized (Dequeue):\n  ");
    while (queue) {
        MToken* tok = queue.dequeue();
        stack.push(tok);
        tok->print();
        printf(" ");
    }
    printf("\n");

    printStack(stack);

    return 0;
}