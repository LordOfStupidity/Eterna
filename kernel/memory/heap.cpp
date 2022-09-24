#include <cstddef>
#include <cstdint>
#include <cstr.hpp>
#include <debug.hpp>
#include <memory/common.hpp>
#include <memory/heap.hpp>
#include <memory/memory.hpp>
#include <memory/paging.hpp>
#include <memory/physical_memory_manager.hpp>
#include <memory/virtual_memory_manager.hpp>
#include <string.hpp>

#define DEBUG_HEAP

void* sHeapStart{nullptr};
void* sHeapEnd{nullptr};
HeapSegmentHeader* sLastHeader{nullptr};

void HeapSegmentHeader::combine_forward() {
    // can't combine nothing
    if (next == nullptr)
        return;

    // don't combine a header that is in use.
    if (next->free == false)
        return;

    // update last header address if it is being changed.
    if (next == sLastHeader)
        sLastHeader = this;

    // set next next segment last to this segment.
    if (next->next != nullptr)
        next->next->last = this;

    length = length + next->length + sizeof(HeapSegmentHeader);
    next = next->next;
}

void HeapSegmentHeader::combine_backward() {
    if (last == nullptr)
        return;
    if (!last->free)
        return;

    last->combine_forward();
}

HeapSegmentHeader* HeapSegmentHeader::split(uint64_t splitLength) {
    if (splitLength + sizeof(HeapSegmentHeader) > length)
        return nullptr;
    if (splitLength < 8)
        return nullptr;

    // Length of segment that is leftover after creating new header of `splitLength` length.
    uint64_t splitSegmentLength =
        length - splitLength - sizeof(HeapSegmentHeader);
    if (splitSegmentLength < 8)
        return nullptr;

    // Position of header that is newly created within middle of `this` header.
    HeapSegmentHeader* splitHeader =
        (HeapSegmentHeader*)((uint64_t)this + sizeof(HeapSegmentHeader) +
                             splitLength);
    if (next) {
        // set next segment's last segment to the new segment
        next->last = splitHeader;
        // set new segment's next segment.
        splitHeader->next = next;
    } else {
        splitHeader->next = nullptr;
        sLastHeader = splitHeader;
    }

    // Set current segment next to newly inserted segment.
    next = splitHeader;
    // set new segment's last segment to this segment.
    splitHeader->last = this;
    // set new length of segments.
    length = splitLength;

    splitHeader->length = splitSegmentLength;
    splitHeader->free = free;

    return this;
}

void init_heap() {
    uint64_t numBytes = HEAP_INITIAL_PAGES * PAGE_SIZE;

    for (uint64_t i = 0; i < numBytes; i += PAGE_SIZE) {
        // Map virtual heap position to physical memory address returned by page frameallocator
        Memory::map((void*)((uint64_t)HEAP_VIRTUAL_BASE + i),
                    Memory::request_page(),
                    (uint64_t)Memory::PageTableFlag::Present |
                        (uint64_t)Memory::PageTableFlag::ReadWrite |
                        (uint64_t)Memory::PageTableFlag::Global);
    }

    sHeapStart = (void*)HEAP_VIRTUAL_BASE;
    sHeapEnd = (void*)((uint64_t)sHeapStart + numBytes);
    HeapSegmentHeader* firstSegment = (HeapSegmentHeader*)HEAP_VIRTUAL_BASE;

    // Actual length of free memory has to take into account header.
    firstSegment->length = numBytes - sizeof(HeapSegmentHeader);
    firstSegment->next = nullptr;
    firstSegment->last = nullptr;
    firstSegment->free = true;
    sLastHeader = firstSegment;
    dbgmsg(
        "[HEAP]: \033[32mInitialized\033[0m\r\n"
        "  Virtual Address: %x thru %x\r\n"
        "  Size: $ull\r\n"
        "\r\n",
        sHeapStart, sHeapEnd, numBytes);
    // heap_print_debug();
}

void expand_heap(uint64_t numBytes) {
#ifdef DEBUG_HEAP
    dbgmsg("[HEAP]: Expanding by %ull bytes\r\n", numBytes);
#endif
    uint64_t numPages = (numBytes / PAGE_SIZE) + 1;
    // Round byte count to page-aligned boundary.
    numBytes = numPages * PAGE_SIZE;
    // Get address of new header at the end of the heap.
    HeapSegmentHeader* extension = (HeapSegmentHeader*)sHeapEnd;

    // Allocate and map a page in memory for new header.
    for (uint64_t i = 0; i < numPages; ++i) {
        Memory::map(sHeapEnd, Memory::request_page(),
                    (uint64_t)Memory::PageTableFlag::Present |
                        (uint64_t)Memory::PageTableFlag::ReadWrite |
                        (uint64_t)Memory::PageTableFlag::Global);
        sHeapEnd = (void*)((uint64_t)sHeapEnd + PAGE_SIZE);
    }

    extension->free = true;
    extension->last = sLastHeader;

    sLastHeader->next = extension;
    sLastHeader = extension;

    extension->next = nullptr;
    extension->length = numBytes - sizeof(HeapSegmentHeader);

    // After expanding, combine with the previous segment (Decrease fragmentation).
    extension->combine_backward();
}

void* malloc(uint64_t numBytes) {
    // can't allocate nothing
    if (numBytes == 0)
        return nullptr;

    // Round numBytes to 64-bit (8-byte) aligned number.
    if (numBytes % 8 > 0) {
        numBytes -= (numBytes % 8);
        numBytes += 8;
    }

#ifdef DEBUG_HEAP
    dbgmsg("[HEAP]: malloc() -- numBytes=%ull\r\n", numBytes);
#endif  // DEBUG_HEAP

    // start looking for a free segment at the start of the heap.
    HeapSegmentHeader* current = (HeapSegmentHeader*)sHeapStart;

    while (true) {
        if (current->free) {
            if (current->length > numBytes) {
                if (HeapSegmentHeader* split = current->split(numBytes)) {
                    split->free = false;
#ifdef DEBUG_HEAP
                    dbgmsg("  Made split.\r\n");
                    heap_print_debug();
#endif  // DEBUG_HEAP
                    return (void*)((uint64_t)split + sizeof(HeapSegmentHeader));
                }
            } else if (current->length == numBytes) {
                current->free = false;
#ifdef DEBUG_HEAP
                dbgmsg("  Found exact match.\r\n");
                heap_print_debug();
#endif  // DEBUG_HEAP
                return (void*)((uint64_t)current + sizeof(HeapSegmentHeader));
            }
        }

        if (current->next == nullptr)
            break;

        current = current->next;
    }

    /**
     * If this point is reached:
     * 1. No free segment of size has been found
     * 2. All existing segments have been searched
     * From here, we must allocate more memory for the heap (expand it),
     *      then do the same search once again. There is some optimization to be done here.
     */
    expand_heap(numBytes);

    return malloc(numBytes);
}

void free(void* address) {
    HeapSegmentHeader* segment =
        (HeapSegmentHeader*)((uint64_t)address - sizeof(HeapSegmentHeader));
#ifdef HEAP_DEBUG
    dbgmsg("[HEAP]: free() -- address=%x, numBytes=%ull\r\n", address,
           segment->length);
#endif  // HEAP_DEBUG
    segment->free = true;
    segment->combine_forward();
    segment->combine_backward();
#ifdef HEAP_DEBUG
    heap_print_debug();
#endif  // HEAP_DEBUG
}

void heap_print_debug_starchart() {
    // One character per 64 bytes of heap.
    constexpr uint8_t characterGranularity = 64;

    uint64_t heapSize = (uint64_t)(sHeapEnd) - (uint64_t)(sHeapStart);
    uint64_t totalChars = heapSize / characterGranularity + 1;
    uint8_t* out = new uint8_t[totalChars];

    memset(out, 0, totalChars);

    uint64_t freeLeftover = 0;
    uint64_t usedLeftover = 0;
    uint64_t offset = 0;

    HeapSegmentHeader* it = (HeapSegmentHeader*)sHeapStart;

    do {
        if (offset >= totalChars)
            break;

        uint64_t segmentSize = it->length + sizeof(HeapSegmentHeader);
        uint64_t delta = segmentSize % characterGranularity;
        uint64_t numChars = segmentSize / characterGranularity;
        if (it->free) {
            freeLeftover += delta;
            if (freeLeftover > characterGranularity) {
                out[offset] = '_';
                offset++;
                freeLeftover -= characterGranularity;
            }
        } else {
            usedLeftover += delta;
            if (usedLeftover > characterGranularity) {
                out[offset] = '*';
                offset++;
                usedLeftover -= characterGranularity;
            }
        }
        char c = it->free ? '_' : '*';
        memset(&out[offset], c, numChars);

        offset += numChars;
        it = it->next;
    } while (it != nullptr);

    String heap_visualization((const char*)out);

    dbgmsg_s("Heap (64b per char): ");
    dbgrainbow(heap_visualization, ShouldNewline::Yes);
    dbgmsg_s("\r\n");
}

void heap_print_debug() {
    uint64_t heapSize = (uint64_t)(sHeapEnd) - (uint64_t)(sHeapStart);

    dbgmsg(
        "[Heap]: Debug information:\r\n"
        "  Size:   %ull\r\n"
        "  Start:  %x\r\n"
        "  End:    %x\r\n"
        "  Regions:\r\n",
        heapSize, sHeapStart, sHeapEnd);

    uint64_t i = 0;
    uint64_t usedCount = 0;
    float usedSpaceEfficiency = 0.0f;
    HeapSegmentHeader* it = (HeapSegmentHeader*)sHeapStart;

    while (it) {
        float efficiency =
            (float)it->length / (float)(it->length + sizeof(HeapSegmentHeader));

        dbgmsg(
            "    Region %ull:\r\n"
            "      Free:   %s\r\n"
            "      Length: %ull (%ull) %%f\r\n"
            "      Header Address:  %x\r\n"
            "      Payload Address: %x\r\n",
            i, to_string(it->free), it->length,
            it->length + sizeof(HeapSegmentHeader), 100.0f * efficiency, it,
            (uint64_t)(it) + sizeof(HeapSegmentHeader));

        if (!it->free) {
            usedSpaceEfficiency += efficiency;
            usedCount++;
        }

        ++i;
        it = it->next;
    };

    dbgmsg("\r\n");

    dbgmsg(
        "Heap Metadata vs Payload ratio in used regions (lower is better): "
        "%%f\r\n\r\n",
        100.0f * (1.0f - usedSpaceEfficiency / (float)usedCount));

    heap_print_debug_starchart();
}

void heap_print_debug_summed() {
    uint64_t heapSize = (uint64_t)(sHeapEnd) - (uint64_t)(sHeapStart);

    dbgmsg(
        "[Heap]: Debug information:\r\n"
        "  Size:   %ull\r\n"
        "  Start:  %x\r\n"
        "  End:    %x\r\n"
        "  Regions:\r\n",
        heapSize, sHeapStart, sHeapEnd);

    float usedSpaceEfficiency = 0.0f;
    uint64_t i = 0;
    uint64_t usedCount = 0;
    HeapSegmentHeader* it = (HeapSegmentHeader*)sHeapStart;

    while (it) {
        HeapSegmentHeader* start_it = it;
        uint64_t payload_total = it->length;
        uint64_t total_length = it->length + sizeof(HeapSegmentHeader);
        float efficiency =
            (float)it->length / (float)(it->length + sizeof(HeapSegmentHeader));

        if (!it->free) {
            usedSpaceEfficiency += efficiency;
            ++usedCount;
        }

        uint64_t start_i = i;
        bool free = it->free;

        while ((it = it->next)) {
            ++i;

            if (it->free != free) {
                break;
            }

            payload_total += it->length;
            total_length += it->length + sizeof(HeapSegmentHeader);
            float localEfficiency =
                (float)it->length /
                (float)(it->length + sizeof(HeapSegmentHeader));
            efficiency += localEfficiency;

            if (!it->free) {
                usedSpaceEfficiency += localEfficiency;
                ++usedCount;
            }
        }

        if (i - start_i == 1 || i - start_i == 0) {
            dbgmsg("    Region %ull:\r\n", start_i);
        } else {
            dbgmsg("    Region %ull through %ull:\r\n", start_i, i - 1);
        }

        efficiency = efficiency / (i - start_i ? i - start_i : 1);

        dbgmsg(
            "      Free:          %s\r\n"
            "      Length:        %ull (%ull) %%f\r\n"
            "      Start Address: %x\r\n",
            to_string(free), payload_total, total_length, 100.0f * efficiency,
            start_it);
    };

    dbgmsg("\r\n");
    dbgmsg(
        "Heap Metadata vs Payload ratio in used regions (lower is better): "
        "%%f\r\n\r\n",
        100.0f * (1.0f - (usedSpaceEfficiency / (float)usedCount)));

    heap_print_debug_starchart();
}

void* operator new(uint64_t numBytes) {
    return malloc(numBytes);
}
void* operator new[](uint64_t numBytes) {
    return malloc(numBytes);
}
void operator delete(void* address) noexcept {
    return free(address);
}
void operator delete[](void* address) noexcept {
    return free(address);
}

void operator delete(void* address, uint64_t unused) {
    (void)unused;
    return free(address);
}
void operator delete[](void* address, uint64_t unused) {
    (void)unused;
    return free(address);
}
