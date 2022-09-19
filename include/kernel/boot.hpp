#ifndef _BOOT_HPP
#define _BOOT_HPP

#include <renderer/renderer.hpp>
#include <memory/efi_memory.hpp>
#include <cstdint>
#include <cstddef>

struct BootInfo {
    Framebuffer* framebuffer;
    PSF1_FONT* font;
    EFI_MEMORY_DESCRIPTOR* map;
    uint64_t mapSize;
    uint64_t mapDescSize;
    void* RSDP;
};

#endif // !_BOOT_HPP