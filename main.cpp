#include <cassert>
#include <iostream>
#include <cstring>

#include "types.h"

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
public:
    enum class Type { N, INTEGER, OPERATOR, GROUP_OPEN, GROUP_CLOSE };
private:
    union { int v_int; MOperator v_operator; };
    Type type; // N is NULL
public:
    MToken() : type(Type::N) { } // null/empty/whitespace
    MToken(int v) : type(Type::INTEGER), v_int(v) { } // numeral constants
    MToken(MOperator o) : type(Type::OPERATOR), v_operator(o) { } // operators
    MToken(bool o) : type(o ? Type::GROUP_OPEN : Type::GROUP_CLOSE) { } // group

    void int_digit_append(int num) {
        if (!is_int()) return;
        if (num < 0) num *= -1;
        v_int *= 10;
        v_int += num;
    }

    bool is_null() const { return type == Type::N; }
    bool is_int() const { return type == Type::INTEGER; }
    bool is_operator() const { return type == Type::OPERATOR; }
    bool is_group() const { return type == Type::GROUP_OPEN || type == Type::GROUP_CLOSE; }

    int value_int() const { return v_int; }
    MOperator value_operator() const { return v_operator; }
    bool value_group_opener() const { return type == Type::GROUP_OPEN; }
    bool value_group_closer() const { return type == Type::GROUP_CLOSE; }

    Type toktype() const { return type; }

    void print() const {
        switch (type) {
            case Type::N: printf("NULLTKN"); return;
            case Type::INTEGER: printf("%i", v_int); return;
            case Type::OPERATOR: printf("%s", op_str(v_operator)); return;
            case Type::GROUP_OPEN: printf("("); return;
            case Type::GROUP_CLOSE: printf(")"); return;
        }
    }
};

// returns number of MTokens recognized in str
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
    LNode<MToken>* head = q.head;
    while (head) {
        head->data->print();
        printf(" ");
        head = head->next;
    }
    printf("(BACK)\n");
}

void printStack(const Stack<MToken> &s) {
    printf("Size: %i, (TOP)\n  ", s.size());
    LNode<MToken>* head = s.head;
    while (head) {
        head->data->print();
        printf("\n  ");
        head = head->next;
    }
    printf("(BOTTOM)\n");
}

void inlineQueue(const Queue<MToken> &q) {
    LNode<MToken>* head = q.head;
    while (head) {
        head->data->print();
        head = head->next;
    }
}

void inlineStack(const Stack<MToken> &s) {
    LNode<MToken>* head = s.head;
    while (head) {
        head->data->print();
        head = head->next;
    }
}

// Parse from infix to postfix notation in-place.
bool parse_infix_to_postfix(Queue<MToken> &q) {
    Stack<MToken> stack{};
    // https://en.wikipedia.org/wiki/Shunting_yard_algorithm
    printf("Parsing infix to postfix:\n");
    const size_t ct = q.size();
    size_t i = 0;
    while (i++ < ct) {
        MToken* tok = q.dequeue();
        printf("[ ");
        tok->print();
        printf(" ] ");

        // Print labels
        switch (tok->toktype()) {
            case MToken::Type::GROUP_OPEN: printf("group opener"); break;
            case MToken::Type::GROUP_CLOSE: printf("group closer"); break;
            case MToken::Type::INTEGER: printf("integer constant"); break;
            case MToken::Type::OPERATOR: printf("math operator"); break;
            default: assert(false); return false;
        }

        printf("\n  ");

        // Token behaviour
        switch (tok->toktype()) {
            case MToken::Type::INTEGER:
                q.enqueue(tok);
                break;
            case MToken::Type::OPERATOR:
                {
                    const MOpSpec& tokSpec = op_spec(tok->value_operator());
                    while (stack) {
                        const MToken* peek = stack.peek();
                        if (peek->is_group()) {
                            if (peek->value_group_opener()) break;
                        }
                        else if (peek->is_operator()) {
                            const MOpSpec& peekSpec = op_spec(peek->value_operator());
                            if (peekSpec.precedence > tokSpec.precedence) {
                                q.enqueue(stack.pop()); // Pop and enqueue
                            }
                            else if (peekSpec.precedence == tokSpec.precedence && tokSpec.assoc_left) {
                                q.enqueue(stack.pop());
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            printf("BAD EQUATION!\n");
                            return false;
                        }
                    }
                    stack.push(tok);
                }
                break;
            case MToken::Type::GROUP_OPEN:
                stack.push(tok);
                break;
            case MToken::Type::GROUP_CLOSE:
                {
                    // printf("Found closer, searching for opener...\n");
                    while (stack) {
                        const MToken* peek = stack.peek();
                        // printf("  ");
                        // peek->print();
                        if (peek->is_group() && peek->value_group_opener()) {
                            stack.pop();
                            // TODO: Add "if (tok.is_function) pop+enqueue" here!
                            // printf("  Found closer! Breaking...\n");
                            break;
                        }
                        else {
                            q.enqueue(stack.pop());
                            // printf("  Inner!");
                        }
                    }
                }
                break;
            default:
                printf("BAD CASE!\n");
                assert(false);
                return false;
        }

        printf("    Output: ");
        inlineQueue(q);
        printf("  Stack: ");
        inlineStack(stack);
        printf("\n\n");
    }
    while (stack) q.enqueue(stack.pop());
    assert(stack.size() == 0);
    printf("\n");

    return true;
}

bool build_binary_expression_tree(Queue<MToken> &q, BTree<MToken> &bt) {
    // Construct a queue of BNodes and evaluate them!
    return false;
} 

void print_binary_expression_tree(BTree<MToken> &bt) {
    // Unsure how to go about this.
    printf("TODO: print_binary_expression_tree!\n");
}

int main(int argc, const char* argv[]) {
    if (argc < 2) { 
        printf("Must specify an expression!\n");
        return 1;
    }

    const char* expr = argv[1];
    printf("Raw: \"%s\"\n", expr);

    Queue<MToken> queue{};
    tokenize(expr, queue);

    printf("Tokenized: ");
    inlineQueue(queue);
    printf("\n");

    parse_infix_to_postfix(queue);

    printf("Output (Postfix Notation):\n");
    printQueue(queue);

    // TODO: Add evaluation
    // TODO: Add ability to convert to abstract syntax tree, infix, or prefix notation.

    return 0;
}