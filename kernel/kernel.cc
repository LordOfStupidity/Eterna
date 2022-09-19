#include <cstdint>
#include <kernel/kernel.hpp>
#include <kernel/kstage1.hpp>
#include <interrupts/interrupts.hpp>
#include <cstr.hpp>
#include <kernel/boot.hpp>
#include <debug.hpp>
#include <renderer/renderer.hpp>
#include <string.hpp>
#include <memory/heap.hpp>
#include <memory/common.hpp>
#include <memory/virtual_memory_manager.hpp>
#include <memory/physical_memory_manager.hpp>
#include <math.hpp>
#include <uart.hpp>

void print_memory_info(Vector2<uint64_t>& position) {
    uint32_t startOffset = position.x;
    uint64_t totalRAM = Memory::total_ram();
    uint64_t freeRAM = Memory::free_ram();
    uint64_t usedRAM = Memory::used_ram();
    
    gRend.puts(position, "Memory Info:");
    
    gRend.crlf(position, startOffset);
    
    gRend.puts(position, "|- Total RAM: ");
    gRend.puts(position, to_string(TO_MiB(totalRAM)));
    gRend.puts(position, " MiB (");
    gRend.puts(position, to_string(TO_KiB(totalRAM)));
    gRend.puts(position, " KiB)");
    
    gRend.crlf(position, startOffset);
    
    gRend.puts(position, "|- Free RAM: ");
    gRend.puts(position, to_string(TO_MiB(freeRAM)));
    gRend.puts(position, " MiB (");
    gRend.puts(position, to_string(TO_KiB(freeRAM)));
    gRend.puts(position, " KiB)");
    
    gRend.crlf(position, startOffset);
    
    gRend.puts(position, "`- Used RAM: ");
    gRend.puts(position, to_string(TO_MiB(usedRAM)));
    gRend.puts(position, " MiB (");
    gRend.puts(position, to_string(TO_KiB(usedRAM)));
    gRend.puts(position, " KiB)");
    
    gRend.crlf(position, startOffset);
}

extern "C" void kmain(BootInfo* bInfo) {
    // The heavy lifting is done within the kstage1 function
    kstage1(bInfo);
    dbgmsg_s("\r\n\033[1;33m!===--- You have now booted into Eterna ---===!\033[0m\r\n");
    
    // Clear + swap screen (ensure known state: blank).
    gRend.clear(0x00000000);
    gRend.swap();
    
    const char* MIT = "Eterna  Copyright (C) 2022, Contributors To Eterna.";
    
    // To seriaL
    UART::out(MIT);
    UART::out("\r\n\r\n");
    
    // To screen
    Vector2<uint64_t> drawPosition = { 0, 0 };
    gRend.BackgroundColor = 0xffffffff;
    gRend.puts(drawPosition, MIT, 0x00000000);
    gRend.BackgroundColor = 0x00000000;
    gRend.crlf(drawPosition);
    gRend.puts(drawPosition, "Do a barrel roll!");
    gRend.crlf(drawPosition);
    gRend.swap({0, 0}, {80000, gRend.Font->PSF1_Header->CharacterSize * 2u});

    uint32_t debugInfoX = gRend.Target->PixelWidth - 300;

    while(true) {
        drawPosition = {debugInfoX, 0};

        // Print Memory Info
        gRend.crlf(drawPosition, debugInfoX);
        print_memory_info(drawPosition);

        // Update top right corner of screen.
        gRend.swap({debugInfoX, 0}, {80000, 400});
    }

    // Halt loop(kernel inactive)
    while(true)
        asm("hlt");
}