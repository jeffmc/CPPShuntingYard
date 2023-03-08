// All nodes keep references to original data.

// LINEAR TYPES HERE:
template <typename NT>
struct LNode { // Linear-node
    NT* data;
    LNode* next = nullptr;

    LNode(NT* ptr) : data(ptr) {} // Node takes ownership of the data

    template <typename... Args>
    LNode(Args&&... args) {
        data = new NT(std::forward<Args>(args)...);
    }
    ~LNode() { if (data) delete data; } // Node owns the data it is constructed with unless reassigned to different pointer/nullptr.
};

template <typename T>
struct Queue {
    LNode<T>* head = nullptr;
    // friend void printQueue(const Queue<T>&&);

    operator bool() const { return head != nullptr; }

    void enqueue(T* t) // Queue takes ownership of the pointer/memory.
    {
        LNode<T>* nn = new LNode<T>(t);
        if (!head) {
            head = nn;
            return;
        }
        LNode<T>* ptr = head;
        while (ptr->next) ptr = ptr->next;
        ptr->next = nn;
    }
    
    size_t size() const 
    {
        size_t sz = 0;
        LNode<T>* n = head;
        while (n) {
            ++sz;
            n = n->next;
        }
        return sz;
    }

    T* dequeue() // Gives ownership of pointer/memory back to user.
    {
        T* val = head->data;
        head->data = nullptr;
        LNode<T>* oldHead = head;
        head = oldHead->next;
        delete oldHead;
        return val; 
    }
};

template <typename T>
struct Stack {
    LNode<T>* head = nullptr;
    // friend void printStack(const Stack&&);

    size_t size() const 
    {
        size_t sz = 0;
        LNode<T>* n = head;
        while (n) {
            ++sz;
            n = n->next;
        }
        return sz;
    }

    operator bool() const { return head != nullptr; }

    void push(T* t) // User gives ownership of memory to the stack.
    { 
        LNode<T>* nn = new LNode<T>(t);
        nn->next = head;
        head = nn;
    }

    T* pop() // User takes back ownership of pointer
    {
        LNode<T>* newHead = head->next;
        T* tptr = head->data;
        head->data = nullptr;
        delete head;
        head = newHead;
        return tptr;
    }
    
    T* peek() // User DOES NOT take ownership of pointer
    {
        return head->data;
    }
};

template <typename NT>
struct BNode { // Binary tree node, has 2 children.
    NT* data;
    BNode *left = nullptr, *right = nullptr;
    
    BNode(NT* ptr) : data(ptr) {} // Node takes ownership of the data

    template <typename... Args>
    BNode(Args&&... args) {
        data = new NT(std::forward<Args>(args)...);
    }
    ~BNode() { if (data) delete data; } // Node owns the data it is constructed with unless reassigned to different pointer/nullptr.

    bool has_children() const { return left != nullptr || right != nullptr; }
    bool is_leaf() const { return left == nullptr && right == nullptr; }
};

// Binary tree implementation
template <typename T>
struct BTree {
    BNode<T>* head = nullptr;
    
};