// Since these are just structures for tokens, I built the types to copy values of data types
// rather than keep references/pointers to originals.

template <typename NT>
struct Node {
    NT* data;
    Node* next = nullptr;

    Node(NT* ptr) : data(ptr) {}

    template <typename... Args>
    Node(Args&&... args) {
        data = new NT(std::forward<Args>(args)...);
    }
    ~Node() { if (data) delete data; } // Node owns the data it is constructed with unless reassigned to different pointer/nullptr.
};

template <typename T>
struct Queue {
    Node<T>* head = nullptr;
    // friend void printQueue(const Queue<T>&&);

    operator bool() const { return head != nullptr; }

    void enqueue(T* t) // Queue takes ownership of the pointer/memory.
    {
        Node<T>* nn = new Node<T>(t);
        if (!head) {
            head = nn;
            return;
        }
        Node<T>* ptr = head;
        while (ptr->next) ptr = ptr->next;
        ptr->next = nn;
    }
    
    size_t size() const 
    {
        size_t sz = 0;
        Node<T>* n = head;
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
        Node<T>* oldHead = head;
        head = oldHead->next;
        delete oldHead;
        return val; 
    }
};

template <typename T>
struct Stack {
    Node<T>* head = nullptr;
    // friend void printStack(const Stack&&);

    size_t size() const 
    {
        size_t sz = 0;
        Node<T>* n = head;
        while (n) {
            ++sz;
            n = n->next;
        }
        return sz;
    }

    operator bool() const { return head != nullptr; }

    void push(T* t) // User gives ownership of memory to the stack.
    { 
        Node<T>* nn = new Node<T>(t);
        nn->next = head;
        head = nn;
    }

    T* pop() // User takes back ownership of pointer
    {
        Node<T>* newHead = head->next;
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