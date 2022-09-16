#ifndef _REGION_HPP
#define _REGION_HPP

#include <stdint.h>
namespace Memory {
    class Region {
    public:
        Region(void* baseAddress, uint64_t length) : BaseAddress(baseAddress), Length(length) {}

        Region(uint64_t baseAddress, uint64_t length)
            : BaseAddress((void*)baseAddress), Length(length) {}

        void* begin() { return BaseAddress; }
        void* end() { return (void*)((uint64_t)BaseAddress + Length); }

        // NOTE: length returns amount of apges in region, not bytes!
        uint64_t lengt() { return Length; }

        // Move this region to a specified address(rebase).
        void move_region(void* address) { BaseAddress = address; }

        // Grow the current region by a given amount of pages.
        void grow_region(uint64_t amount) { Length += amount; }

    private:
        // The byte address of the beginning of the memory region.
        void* BaseAddress{nullptr};

        // The length of the contiguous memory region starting at the base address, in pages.
        uint64_t Length;
    };
}  // namespace Memory

#endif  // !_REGION_HPP