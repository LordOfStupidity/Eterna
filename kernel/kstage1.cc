#include <arch/x86_64/gdt.hpp>
#include <bitmap.hpp>
#include <cstddef>
#include <cstdint>
#include <cstr.hpp>
#include <debug.hpp>
#include <int.hpp>
#include <interrupts/idt.hpp>
#include <interrupts/interrupts.hpp>
#include <io/io.hpp>
#include <kernel/boot.hpp>
#include <kernel/kstage1.hpp>
#include <link_definitions.hpp>
#include <memory/common.hpp>
#include <memory/efi_memory.hpp>
#include <memory/heap.hpp>
#include <memory/memory.hpp>
#include <memory/paging.hpp>
#include <memory/physical_memory_manager.hpp>
#include <memory/region.hpp>
#include <memory/virtual_memory_manager.hpp>
#include <panic/panic.hpp>
#include <renderer/renderer.hpp>
#include <uart.hpp>

uint8_t idt_storage[0x1000];

void prepare_interrupts() {
    // Remap PIC chip IRQs out of the way of General software Exceptions.
    remap_pic();

    // Create Interrupt Descriptor Table
    gIDT = IDTR(0x0fff, (uint64_t)&idt_storage[0]);

    // Populate Table
    gIDT.install_handler((uint64_t)uart_com1_handler, PIC_IRQ4);
    gIDT.install_handler((uint64_t)divide_by_zero_handler, 0x00);
    gIDT.install_handler((uint64_t)double_fault_handler, 0x08);
    gIDT.install_handler((uint64_t)stack_segment_fault_handler, 0x0c);
    gIDT.install_handler((uint64_t)general_protection_fault_handler, 0x0d);
    gIDT.install_handler((uint64_t)page_fault_handler, 0x0e);
    gIDT.install_handler((uint64_t)simd_exception_handler, 0x13);

    gIDT.flush();
}

void draw_boot_gfx() {
    Vector2<uint64_t> drawPosition = {0, 0};
    gRend.puts(drawPosition,
               "<<<!===--- You are now booting into Eterna ---===!>>");

    // Draw a face
    drawPosition = {420, 420};

    // left eye
    gRend.drawRect(drawPosition, {42, 42}, 0xff00ffff);

    // left pupil
    drawPosition = {440, 440};
    gRend.drawRect(drawPosition, {20, 20}, 0xffff0000);

    // right eye
    drawPosition = {520, 420};
    gRend.drawRect(drawPosition, {42, 42}, 0xff00ffff);

    // right pupil
    drawPosition = {540, 440};
    gRend.drawRect(drawPosition, {20, 20}, 0xffff0000);

    // mouth
    drawPosition = {400, 520};
    gRend.drawRect(drawPosition, {182, 20}, 0xff00ffff);

    gRend.swap();
}

// FXSAVE/FXRSTOR instructions require a pointer to a 512-byte region of memory before use.
uint8_t fxsave_region[512] __attribute__((aligned(16)));

void kstage1(BootInfo* bInfo) {
    /**
     * @brief This function is monstrous, so the functionality is outlined here.
     *    booting - Disable interrupts (if they weren't already)
     *     - Ensure BootInfo pointer is valid (non-null)
     * x86 - Load Global Descriptor Table
     * x86 - Load Interrupt Descriptor Table
     *     - Prepare UART serial communications driver
     *     - Prepare physical/virtual memory
     *       - Initialize Physical Memory Manager
     *       - Initialize Virtual Memory Manager
     *       - Prepare the heap (`new` and `delete`)
     *     - Prepare Real Time Clock (RTC)
     *     - Setup graphical renderers  -- these will change, and soon
     *       - BasicRenderer      -- drawing pixels to linear framebuffer
     *       - BasicTextRenderer  -- draw keyboard input on screen,
     *                               keep track of text cursor, etc
     *     - Determine and cache information about CPU(s)
     *     - Initialize ACPI
     *     - Enumerate PCI
     *     - Prepare non-PCI devices
     *       - High Precision Event Timer (HPET)
     *       - PS2 Mouse
     *     - Prepare Programmable Interval Timer (PIT)
     * x86 - Setup TSS
     *     - Setup scheduler
     * x86 - Clear (IRQ) interrupt masks in PIC for used interrupts
     *     - Print information about the system to serial output
     *     - Enable interrupts
     *
     * x86 = The step is inherently x86-only (not implementation based).
     */

    // Disable interrupts while doing sensitive operations.
    asm("cli");

    // Don't even attempt to boot unless boot info exists.
    if (bInfo == nullptr)
        while (true)
            asm("hlt");

    /**
     * @brief Tell x86_64 CPU where the GDT is located by
     *  populating and loading a GDT descriptor.
     *  The global descriptor table contains information about
     *  memory segments (like privilege level of executing code,
     *  or privilege level needed to access data).
     */
    setup_gdt();
    gGDTD.Size = sizeof(GDT) - 1;
    gGDTD.Offset = V2P((uint64_t)&gGDT);
    LoadGDT((GDTDescriptor*)V2P(&gGDTD));

    // Prepare Interrupt Descriptor Table.
    prepare_interrupts();

    // setup serial communications chip to allow for debug messages ASAP
    UART::initialize();
    dbgmsg_s(
        "\r\n"
        "!===--- You are now booting into \033[1;33mEterna\033[0m ---===!\r\n"
        "\r\n");

    // Setup physical memory allocator from EFI memory map
    Memory::init_physical(bInfo->map, bInfo->mapSize, bInfo->mapDescSize);

    // Setup virtual memory (map entire address space as well as kernel).
    Memory::init_virtual();

    // Setup dynamic memory allocation (`new`, `delete`)
    init_heap();

    // Create framebuffer renderer.
    dbgmsg_s("[kstage1]: Setting up Graphics Output Protocol Renderer\r\n");
    gRend = Renderer(bInfo->framebuffer, bInfo->font);
    dbgmsg_s("  \033[32mSetup Successful\033[0m\r\n\r\n");
    draw_boot_gfx();

    // Enable IRQ interrupts that will be used.
    disable_all_interrupts();
    enable_interrupt(IRQ_UART_COM1);

    // Memory::print_efi_memory_map_summed(bInfo->map, bInfo->mapSize, bInfo->mapDescSize);
    // heap_print_debug_summed();
    // Memory::print_debug();

    // ALlow interrupts to trigger.
    dbgmsg_s("[kstage1]: Enabling interrupts\r\n");
    asm("sti");
    dbgmsg_s("[kstage1]: \033[32mInterrupts enabled\033[0m\r\n");
}