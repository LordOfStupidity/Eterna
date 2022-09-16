#ifndef _EFI_MEMORY_HPP
#define _EFI_MEMORY_HPP

#include <cstddef>
#include <cstdint>

struct EFI_MEMORY_DESCRIPTOR {
    uint32_t Type;
    uint32_t Pad;
    void* PhysicalAddress;
    void* VirtualAddress;
    uint64_t NumPages;
    uint64_t Attributes;
};

namespace Memory {
    extern const char* EFI_MEMORY_TYPE_STRINGS[];
}  // namespace Memory

#endif  // !_EFI_MEMORY_HPP