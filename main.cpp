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
struct MOpSpec {
    MOperator enum_val;
    const char* symbol;
    bool assoc_left;
    int precedence;
};
const MOpSpec BADSPEC = MOpSpec{ MOperator::MOD, "BAD_SHIT!", false, -1};
const MOpSpec opSpecs[] = {
    {MOperator::ADD,  "+", true,  0},
    {MOperator::SUB,  "-", true,  0},
    {MOperator::MULT, "*", true,  1},
    {MOperator::DIV,  "/", true,  1},
    {MOperator::PWR,  "^", false, 2},
    {MOperator::MOD,  "%", true,  2},
};
const size_t opCount = sizeof(opSpecs) / sizeof(opSpecs[0]);

const MOpSpec& op_spec(const MOperator op) {
    for (int i=0;i<opCount;++i) if (op == opSpecs[i].enum_val) return opSpecs[i];
    return BADSPEC;
}

const char* op_str(const MOperator op) {
    return op_spec(op).symbol;
    return "UNFINDABLE op_str() CALL!";
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

    int value_int() const { return v_int; }
    MOperator value_operator() const { return v_operator; }
    bool value_group_opener() const { return v_open; }
    bool value_group_closer() const { return !v_open; }

    void print() const {
        switch (type) {
            case Type::N: printf("NULLTKN"); return;
            case Type::INTEGER: printf("%i", v_int); return;
            case Type::OPERATOR: printf("%s", op_str(v_operator)); return;
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
    printf("Size: %i, (TOP)\n  ", s.size());
    Node<MToken>* head = s.head;
    while (head) {
        head->data->print();
        printf("\n  ");
        head = head->next;
    }
    printf("(BOTTOM)\n");
}

void inlineQueue(const Queue<MToken> &q) {
    Node<MToken>* head = q.head;
    while (head) {
        head->data->print();
        head = head->next;
    }
}

void inlineStack(const Stack<MToken> &s) {
    Node<MToken>* head = s.head;
    while (head) {
        head->data->print();
        head = head->next;
    }
}

int main(int argc, const char* argv[]) {
    const char* comb = concat(1,argc,argv);
    printf("Raw: \"%s\"\n", comb);

    Queue<MToken> input{}, output{};
    size_t tknCt = tokenize(comb, input);

    printf("Tokenized: "); 
    inlineQueue(input);
    printf("\n");

    Stack<MToken> stack{};

    printf("Parsing:\n");
    while (input) {
        MToken* tok = input.dequeue();
        printf("[ ");
        tok->print();
        printf(" ]");

        if (tok->is_group()) {
            printf(" %s\n", tok->value_group_opener() ? "opener" : "closer");
        }
        else {
            printf("\n");
        }

        if (tok->is_int()) {
            output.enqueue(tok);
        }
        else if (tok->is_operator()) {
            const MOpSpec& tokSpec = op_spec(tok->value_operator());
            
            while (stack) {
                const MToken* peek = stack.peek();
                if (peek->is_group()) {
                    if (peek->value_group_opener()) break;
                }
                else if (peek->is_operator()) {
                    const MOpSpec& peekSpec = op_spec(peek->value_operator());
                    if (peekSpec.precedence > tokSpec.precedence) {
                        output.enqueue(stack.pop()); // Pop and enqueue
                    }
                    else if (peekSpec.precedence == tokSpec.precedence && tokSpec.assoc_left) {
                        output.enqueue(stack.pop());
                    }
                    else {
                        break;
                    }
                }
                else {
                    printf("BAD EQUATION!\n");
                    break;
                }
            }
            stack.push(tok);
        }
        else if (tok->is_group()) {
            if (tok->value_group_opener()) {
                stack.push(tok);
            }
            else {
                printf("Found closer, searching for opener...\n");
                while (stack) {
                    const MToken* peek = stack.peek();
                    printf("  ");
                    peek->print();
                    if (peek->is_group() && peek->value_group_opener()) {
                        stack.pop();
                        // TODO: Add "if (tok.is_function) pop+enqueue" here!
                        printf("  Found closer! Breaking...\n");
                        break;
                    }
                    else {
                        output.enqueue(stack.pop());
                        printf("  Inner!");
                    }
                    printf("\n");
                }
            }
        }
        else {
            printf("BAD CASE!\n");
        }
        printf("    ");
        inlineQueue(output);
        printf("  ");
        inlineStack(stack);
        printf("\n\n");
    }
    while (stack) output.enqueue(stack.pop());
    printf("\n");

    printf("Output:\n");
    printQueue(output);
    printStack(stack);

    return 0;
}