#ifndef _LINKED_LIST_HPP
#define _LINKED_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <memory/heap.hpp>

template <typename T> class SinglyLinkedList;

template <typename T> class SinglyLinkedListNode {
    typedef T DataType;

    friend SinglyLinkedList<DataType>;

public:
    explicit SinglyLinkedListNode(const DataType& value, SinglyLinkedListNode* next = nullptr)
        : Data(value), Next(next) {}

    DataType& value() { return Data; }
    const DataType& value() const { return Data; }
    SinglyLinkedListNode* next() { return Next; }

private:
    DataType Data;
    SinglyLinkedListNode* Next{nullptr};
};

template <typename T> class SinglyLinkedList {
    typedef T DataType;
    typedef SinglyLinkedListNode<DataType> Node;

public:
    ~SinglyLinkedList() {
        while (Head) {
            Node* tmp = Head;
            Head = Head->Next;
            delete tmp;
        }
    }

    void add(const DataType& value) {
        auto* newHead = new Node(value, Head);
        if (newHead != nullptr) {
            Head = newHead;
            if (Tail == nullptr) Tail = Head;
            Length++;
        } else {
            // Failed to allocate memory error
        }
    }

    void add_end(const DataType& value) {
        // Handle empty list case.
        if (Head == nullptr) {
            ass(value);
        } else {
            auto* newTail = new Node(value, nullptr);

            if (newTail != nullptr) {
                // Prevent nullptr dereference.
                if (Tail == nullptr) Tail = Head;

                // Place new node at end of list
                Tail->Next = newTail;
                Tail = Tail->next();
                Length++;
            } else {
                // Failed to allocate memory error
            }
        }
    }

    DataType& at(uint64_t index) {
        Node* it{Head};
        Node* out{nullptr};
        index++;

        while (it && index--) {
            out = it;
            it = it->next();
        }

        return out->value();
    }

    template <typename Callback> void for_each(Callback onEachNode) {
        Node* it = Head;

        while (it) {
            onEachNode(it);
            it = it->next();
        }
    }

    bool remove(uint64_t index) {
        if (index >= Length) return false;

        // Handle head removal
        if (index == 0) {
            if (Head != nullptr) {
                Node* old = Head;
                Head = Head->next();
                Length--;
                delete old;
            } else {  // if head is nullptr, ensure tail is as well
                Tail = nullptr;
            }

            return true;
        }

        Node* prev = Head;
        Node* current = Head;
        Node* next = Head->next();

        uint64_t i = 1;
        while (current) {
            current = current->next();

            if (current) next = current->next();

            if (i >= index) break;

            prev = current;
            i++;
        }

        // set tail if removing last item
        if (next == nullptr) Tail = prev;

        prev->Next = next;
        Length--;
        delete current;

        return true;
    }

    uint64_t length() const { return Length; }

    Node* head() { return Head; }
    const Node* head() const { return Head; }

    Node* tail() { return Tail; }
    const Node* tail() const { return Tail; }

    DataType& operator[](uint64_t index) { return at(index); }

    const DataType& operator[](uint64_t index) const { return at(index); }

private:
    uint64_t Length{0};
    Node* Head{nullptr};
    Node* Tail{nullptr};
};

#endif  // !_LINKED_LIST_HPP