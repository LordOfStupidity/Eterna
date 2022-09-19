#ifndef _HEAP_HPP
#define _HEAP_HPP

#include <cstdint>
#include <cstddef>

#define HEAP_VIRTUAL_BASE 0xffffffffff000000
#define HEAP_INITIAL_PAGES 1

struct HeapSegmentHeader {
    // Doubly linked list
    HeapSegmentHeader* last {nullptr};
    HeapSegmentHeader* next {nullptr};

    // Data fields
    uint64_t length {0};
    bool free {false};

    // Fragmentation prevention
    void combine_forward();
    void combine_backward();

    // Allocation
    HeapSegmentHeader* split(uint64_t splitLength);
} __attribute__((packed));

void init_heap();

// Enlarge the heap by a given number of bytes, aligned to next-highest page-aligned value
void expand_heap(uint64_t numBytes);

__attribute__((malloc, alloc_size(1))) void* malloc(uint64_t numBytes);
void free(void* address);

void* operator new(uint64_t numBytes);
void* operator new[](uint64_t numBytes);
void operator delete(void* address) noexcept;
void operator delete[](void* address) noexcept;

void operator delete(void* address, uint64_t unused);
void operator delete[](void* address, uint64_t unused);

void heap_print_debug();
void heap_print_debug_summed();

extern void* sHeapStart;
extern void* sHeapEnd;

#endif // !_HEAP_HPP