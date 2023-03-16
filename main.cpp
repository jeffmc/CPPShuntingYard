#include <cassert>
#include <iostream>
#include <cstring>
#include <cmath>

#include "types.h"

enum class MOperator { ADD, SUB, MULT, DIV, PWR, MOD, N };
struct MOpSpec {
    MOperator enum_val;
    const char* symbol;
    bool assoc_left;
    int precedence;
};
const MOpSpec BADSPEC = MOpSpec{ MOperator::N, "BAD_SHIT!", false, -1};
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
    assert(false);
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

// Build binary expression tree from a post-fix notation queue.
BNode<MToken>* build_binary_expression_tree(Queue<MToken> &pq) {
    Stack<BNode<MToken>> bs{};

    while (pq) {
        BNode<MToken>* n = new BNode<MToken>(pq.dequeue());
        switch (n->data->toktype()) {
            case MToken::Type::INTEGER: 
                bs.push(n);
                break;
            case MToken::Type::OPERATOR: 
                {
                    assert(bs.size() >= 2);
                    BNode<MToken>* r_operand = bs.pop();
                    BNode<MToken>* l_operand = bs.pop();
                    n->left = l_operand;
                    n->right = r_operand;
                    bs.push(n);
                }
                break;
            default: 
                printf("Queue should be in post-fix notation!\n"); 
                assert(false); 
                return nullptr;
        }
    }
    assert(bs.size() == 1);
    return bs.pop();
}

double bnode_eval(BNode<MToken>* node) {
    switch (node->data->toktype()) {
        case MToken::Type::INTEGER:
            return (double) node->data->value_int();
        case MToken::Type::OPERATOR: 
            assert(node->full());
            {
                switch (node->data->value_operator()) {
                    case MOperator::ADD:
                        return bnode_eval(node->left) + bnode_eval(node->right);
                    case MOperator::SUB:
                        return bnode_eval(node->left) - bnode_eval(node->right);
                    case MOperator::MULT:
                        return bnode_eval(node->left) * bnode_eval(node->right);
                    case MOperator::DIV:
                        return bnode_eval(node->left) / bnode_eval(node->right);
                    case MOperator::PWR:
                        return pow(bnode_eval(node->left), bnode_eval(node->right));
                    case MOperator::MOD:
                        return remainder(bnode_eval(node->left), bnode_eval(node->right));
                    defualt:
                        printf("BAD CASE!\n");
                        assert(false);
                        return 0.0f;
                }
            };
        default: 
            printf("Queue should be in post-fix notation!\n"); 
            assert(false); 
            return 0.0f;
    }
}

// https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c
void print_bnode(BNode<MToken> *node, const char* prefix, bool isLeft) {
    if( node == nullptr ) return;
    
    printf("%s", prefix);
    printf(isLeft ? "├───" : "└───" );

    // print the value of the node
    node->data->print();
    printf("\n");
    
    char* npref = new char[strlen(prefix)+5];
    strcpy(npref,prefix);
    strcat(npref, isLeft ? "│   " : "    ");

    // enter the next tree level - left and right branch
    print_bnode(node->left, npref, true);
    print_bnode(node->right, npref, false);

}

void print_bnode(BNode<MToken>* node) {
    print_bnode(node, "", false);
}

void print_bst_outfix(BNode<MToken>* node, const bool prefix) 
{
    if (prefix) node->data->print();
    if (node->left) print_bst_outfix(node->left, prefix);
    if (node->right) print_bst_outfix(node->right, prefix);
    if (!prefix) node->data->print();
}

void print_bst_infix(BNode<MToken>* node, int parent_prec) 
{
    const bool is_op = node->data->is_operator();
    int op_prec = -1;
    if (is_op) {
        op_prec = op_spec(node->data->value_operator()).precedence;
    }

    // TODO: Fix this grouping bug with precedence.
    const bool grouping = is_op && parent_prec < op_prec;

    if (grouping) printf("(");
    if (node->left) print_bst_infix(node->left, op_prec);

        node->data->print();

    if (node->right) print_bst_infix(node->right, op_prec);
    if (grouping) printf(")");
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

    printf("Building tree:\n");
    BNode<MToken>* tree = build_binary_expression_tree(queue);
    
    printf("Output (Binary Syntax Tree):\n");
    print_bnode(tree);

    printf("Printing tree in prefix: ");
    print_bst_outfix(tree, true);
    printf("\n");
    printf("Printing tree in postfix: ");
    print_bst_outfix(tree, false);
    printf("\n");
    printf("Printing tree in infix: ");
    print_bst_infix(tree, 1000);
    printf("\n");

    // printf("Evaluation: %f\n", bnode_eval(tree));

    // TODO: Add evaluation
    // TODO: Add ability to convert to abstract syntax tree, infix, or prefix notation.

    printf("Goodbye World!\n");

    return 0;
}