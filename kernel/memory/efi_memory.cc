#include <cstr.hpp>
#include <memory/common.hpp>
#include <memory/efi_memory.hpp>
#include <memory/memory.hpp>
#include <uart.hpp>

namespace Memory {
const char* EFI_MEMORY_TYPE_STRINGS[]{
    "EfiReservedMemoryType",
    "EfiLoaderCode",
    "EfiLoaderData",
    "EfiBootServicesCode",
    "EfiBootServicesData",
    "EfiRuntimeServicesCode",
    "EfiRuntimeServicesData",
    "EfiConventionalMemory",
    "EfiUnusableMemory",
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS",
    "EfiMemoryMappedIO",
    "EfiMemoryMappedIOPortSpace",
    "EfiPalCode",
};

void print_efi_memory_map(EFI_MEMORY_DESCRIPTOR* map, uint64_t mapSize,
                          uint64_t mapDescSize) {
    uint64_t mapEntries = mapSize / mapDescSize;
    for (uint64_t i = 0; i < mapEntries; ++i) {
        EFI_MEMORY_DESCRIPTOR* desc =
            (EFI_MEMORY_DESCRIPTOR*)((uint64_t)map + (i * mapDescSize));
        
        UART::out("\033[36m[MEMORY REGION]: ");
        
        if (desc->Type < 14) {
            UART::out(EFI_MEMORY_TYPE_STRINGS[desc->Type]);
        } else {
            UART::out("\033[0mINVALID TYPE\033[36m");
        }
        
        UART::out("\r\n  Physical Address: 0x");
        UART::out(to_hexstring<void*>(desc->PhysicalAddress));
        UART::out("\r\n  Size: ");
        
        uint64_t sizeKiB = desc->NumPages * PAGE_SIZE / 1024;
        
        UART::out(to_string(sizeKiB / 1024));
        UART::out("MiB (");
        UART::out(to_string(sizeKiB));
        UART::out("KiB)\033[0m\r\n");
    }
}

void print_efi_memory_map_summed(EFI_MEMORY_DESCRIPTOR* map, uint64_t mapSize,
                                 uint64_t mapDescSize) {
    uint64_t mapEntries = mapSize / mapDescSize;
    uint64_t typePageSums[14];
    
    // Zero out sums to ensure a known starting point.
    memset(&typePageSums[0], 0, 14 * sizeof(uint64_t));
    
    for (uint64_t i = 0; i < mapEntries; ++i) {
        EFI_MEMORY_DESCRIPTOR* desc =
            (EFI_MEMORY_DESCRIPTOR*)((uint64_t)map + (i * mapDescSize));
    
        if (desc->Type < 14)
            typePageSums[desc->Type] += desc->NumPages;
    }
    
    for (uint8_t i = 0; i < 14; ++i) {
        UART::out("\033[36m[MEMORY REGION]: ");
        UART::out(EFI_MEMORY_TYPE_STRINGS[i]);
        UART::out("\r\n  Total Size: ");
    
        uint64_t sizeKiB = typePageSums[i] * PAGE_SIZE / 1024;
    
        UART::out(to_string(sizeKiB / 1024));
        UART::out("MiB (");
        UART::out(to_string(sizeKiB));
        UART::out("KiB)\033[0m\r\n");
    }
}

}  // namespace Memory