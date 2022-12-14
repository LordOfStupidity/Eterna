#include <cstdint>
#include <cstr.hpp>
#include <interrupts/interrupts.hpp>
#include <io/io.hpp>
#include <panic/panic.hpp>
#include <renderer/renderer.hpp>
#include <uart.hpp>

void enable_interrupt(uint8_t irq) {
    if (irq > 15)
        return;

    uint16_t port = PIC2_DATA;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        irq -= 8;
    }

    uint8_t value = in8(port) & ~IRQ_BIT(irq);
    out8(port, value);
}

void disable_interrupt(uint8_t irq) {
    if (irq > 15)
        return;

    uint16_t port = PIC2_DATA;
    if (irq < 8)
        port = PIC1_DATA;
    else
        irq -= 8;
    uint8_t value = in8(port) | IRQ_BIT(irq);
    out8(port, value);
}

void disable_all_interrupts() {
    out8(PIC1_DATA, 0);
    out8(PIC2_DATA, 0);
}

__attribute__((no_caller_saved_registers)) inline void end_of_interrupt(
    uint8_t IRQx) {
    if (IRQx >= 8)
        out8(PIC2_COMMAND, PIC_EOI);
    out8(PIC1_COMMAND, PIC_EOI);
}

void cause_div_by_zero(volatile uint8_t one) {
    one /= one - 1;
}

void cause_page_not_present() {
    uint8_t* badAddr = (uint8_t*)0xdeadc0de;
    volatile uint8_t faultHere = *badAddr;
    (void)faultHere;
}

void cause_nullptr_dereference() {
    uint8_t* badAddr = (uint8_t*)nullptr;
    volatile uint8_t faultHere = *badAddr;
    (void)faultHere;
}

void cause_general_protection() {
    uint8_t* badAddr = (uint8_t*)0xdeadbeefb00bface;
    volatile uint8_t faultHere = *badAddr;
    (void)faultHere;
}

// HARDWARE INTERRUPT HANDLERS (IRQs)
// IRQ0: SYSTEM TIMER
__attribute__((interrupt)) void system_timer_handler(InterruptFrame* frame) {}

// IRQ1: PS/2 KEYBOARD
__attribute__((interrupt)) void keyboard_handler(InterruptFrame* frame) {}

// IRQ4: COM1/COM3 Serial Communications Recieved
__attribute__((interrupt)) void uart_com1_handler(InterruptFrame* frame) {
    uint8_t data = UART::read();
    end_of_interrupt(4);
}

/**
 *
 * @note: If register 'C' is not read from inside this handler,
 *         no further interrupts of this type will be sent.
 * @details Status Register `C`:
 *   Bits 0-3: Reserved (do not touch)
 *          4: Update-ended interrupt
 *          5: Alarm interrupt
 *          6: Periodic Interrupt
 *          7: Interrupt Request (IRQ)
 */
__attribute__((interrupt)) void rtc_handler(InterruptFrame* frame) {}

/// IRQ12: PS/2 MOUSE
__attribute__((interrupt)) void mouse_handler(InterruptFrame* frame) {}

/// FAULT INTERRUPT HANDLERS

__attribute__((interrupt)) void divide_by_zero_handler(InterruptFrame* frame) {
    panic(frame, "Divide by zero detected!");
    while (true)
        asm("hlt");
}

enum class PageFaultErrorCode {
    Present = 1 << 0,
    ReadWrite = 1 << 1,
    UserSuper = 1 << 2,
    Reserved = 1 << 3,
    InstructionFetch = 1 << 4,
    ProtectionKeyViolation = 1 << 5,
    ShadowStackAccess = 1 << 6,
    HypervisorManagedLinearAddressTranslation = 1 << 7,
    SoftwareGaurdExtensions = 1 << 15,
};

__attribute__((interrupt)) void page_fault_handler(InterruptFrameError* frame) {
    // Collect faulty address as soon as possible (it may be lost quickly).
    uint64_t address;
    asm volatile("mov %%cr2, %0" : "=r"(address));
    /**
     * US RW P - Description
     * 0  0  0 - Supervisory process tried to read a non-present page entry
     * 0  0  1 - Supervisory process tried to read a page and caused a protection fault
     * 0  1  0 - Supervisory process tried to write to a non-present page entry
     * 0  1  1 - Supervisory process tried to write a page and caused a protection fault
     * 1  0  0 - User process tried to read a non-present page entry
     * 1  0  1 - User process tried to read a page and caused a protection fault
     * 1  1  0 - User process tried to write to a non-present page entry
     * 1  1  1 - User process tried to write a page and caused a protection fault
     */
    bool notPresent =
        (frame->error & (uint64_t)PageFaultErrorCode::Present) == 0;
    if ((frame->error & (uint64_t)PageFaultErrorCode::UserSuper) > 0) {
        if ((frame->error & (uint64_t)PageFaultErrorCode::ReadWrite) > 0) {
            if (notPresent)
                panic(frame,
                      "#PF: User process attempted to write to a page that is "
                      "not "
                      "present");
            else
                panic(frame,
                      "#PF: User process attempted to write to a page and "
                      "caused a "
                      "protection fault");
        } else {
            if (notPresent)
                panic(frame,
                      "#PF: User process attempted to read from a page that is "
                      "not "
                      "present");
            else
                panic(frame,
                      "#PF: User process attempted to read from a page and "
                      "caused a "
                      "protection "
                      "fault");
        }
    } else {
        if ((frame->error & (uint64_t)PageFaultErrorCode::ReadWrite) > 0) {
            if (notPresent)
                panic(frame,
                      "#PF: Supervisor process attempted to write to a page "
                      "that is "
                      "not present");
            else
                panic(frame,
                      "#PF: Supervisor process attempted to write to a page "
                      "and caused a "
                      "protection fault");
        } else {
            if (notPresent)
                panic(frame,
                      "#PF: Supervisor process attempted to read from a page "
                      "that is "
                      "not present");
            else
                panic(
                    frame,
                    "#PF: Supervisor process attempted to read from a page and "
                    "caused a "
                    "protection fault");
        }
    }
    if ((frame->error & (uint64_t)PageFaultErrorCode::InstructionFetch) > 0) {
        UART::out("  Instruction fetch\r\n");
    }

    if ((frame->error & (uint64_t)PageFaultErrorCode::ShadowStackAccess) > 0) {
        UART::out("  Shadow stack access\r\n");
    }

    if ((frame->error &
         (uint64_t)
             PageFaultErrorCode::HypervisorManagedLinearAddressTranslation) >
        0) {
        UART::out("  Hypvervisor-managed linear address translation\r\n");
    }

    if ((frame->error & (uint64_t)PageFaultErrorCode::SoftwareGaurdExtensions) >
        0) {
        UART::out("  Software gaurd extensions\r\n");
    }

    UART::out("  Faulty Address: 0x");
    UART::out(to_hexstring(address));
    UART::out("\r\n");
    Vector2<uint64_t> drawPosition = {PanicStartX, PanicStartY};
    gRend.puts(drawPosition, "Faulty Address: 0x", 0x00000000);
    gRend.puts(drawPosition, to_hexstring(address), 0x00000000);
    gRend.swap({PanicStartX, PanicStartY}, {1024, 128});
    while (true)
        asm("hlt");
}

__attribute__((interrupt)) void double_fault_handler(
    InterruptFrameError* frame) {
    panic(frame, "Double fault detected!");
    while (true) {
        asm("hlt");
    }
}

__attribute__((interrupt)) void stack_segment_fault_handler(
    InterruptFrameError* frame) {
    if (frame->error == 0)
        panic(frame, "Stack segment fault detected (0)");
    else
        panic(frame, "Stack segment fault detected (selector)!");

    if (frame->error & 0b1)
        UART::out("  External\r\n");

    uint8_t table = (frame->error & 0b110) >> 1;
    if (table == 0b00)
        UART::out("  GDT");
    else if (table == 0b01 || table == 0b11)
        UART::out("  IDT");
    else if (table == 0b10)
        UART::out("  LDT");

    UART::out(" Selector Index: ");
    UART::out(to_hexstring(((frame->error & 0b1111111111111000) >> 3)));
    UART::out("\r\n");
}

__attribute__((interrupt)) void general_protection_fault_handler(
    InterruptFrameError* frame) {
    if (frame->error == 0) {
        panic(frame, "General protection fault detected (0)!");
    } else {
        panic(frame, "General protection fault detected (selector)!");
    }

    if (frame->error & 0b1)
        UART::out("  External\r\n");

    uint8_t table = (frame->error & 0b110) >> 1;

    if (table == 0b00) {
        UART::out("  GDT");
    } else if (table == 0b01 || table == 0b11) {
        UART::out("  IDT");
    } else if (table == 0b10) {
        UART::out("  LDT");
    }

    UART::out(" Selector Index: ");
    UART::out(to_hexstring(((frame->error & 0b1111111111111000) >> 3)));
    UART::out("\r\n");

    while (true)
        asm("hlt");
}

__attribute__((interrupt)) void simd_exception_handler(InterruptFrame* frame) {
    /** 
     * @note: Data about why exception occurred can be found in MXCSR register.
     * MXCSR Register breakdown:
     * 0b00000000
     *          =   -- invalid operation flag
     *         =    -- denormal flag
     *        =     -- divide-by-zero flag
     *       =      -- overflow flag
     *      =       -- underflow flag
     *     =        -- precision flag
     *    =         -- denormals are zeros flag
     */
    uint32_t mxcsr{0};
    asm volatile("ldmxcsr %0" ::"m"(mxcsr));
    if (mxcsr & 0b00000001)
        panic(frame, "SIMD fault detected (Invalid Operation)!");
    else if (mxcsr & 0b00000010)
        panic(frame, "SIMD fault detected (Denormal)!");
    else if (mxcsr & 0b00000100)
        panic(frame, "SIMD fault detected (Divide by Zero)!");
    else if (mxcsr & 0b00001000)
        panic(frame, "SIMD fault detected (Overflow)!");
    else if (mxcsr & 0b00010000)
        panic(frame, "SIMD fault detected (Underflow)!");
    else if (mxcsr & 0b00100000)
        panic(frame, "SIMD fault detected (Precision)!");
    else if (mxcsr & 0b01000000)
        panic(frame, "SIMD fault detected (Denormals are Zero)!");
    else
        panic(frame, "Unknown SIMD fault");
    while (true)
        asm("hlt");
}

void remap_pic() {
    // SAVE INTERRUPT MASKS.
    uint8_t parentMasks;
    uint8_t childMasks;
    parentMasks = in8(PIC1_DATA);
    
    io_wait();
    childMasks = in8(PIC2_DATA);
    io_wait();
    
    // INITIALIZE BOTH CHIPS IN CASCADE MODE.
    out8(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    out8(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // SET VECTOR OFFSET OF MASTER PIC.
    //   This allows software to throw low interrupts as normal (0-32)
    //     without triggering an IRQ that would normally be attributed to hardware.
    out8(PIC1_DATA, PIC_IRQ_VECTOR_OFFSET);
    io_wait();
    
    // SET VECTOR OFFSET OF SLAVE PIC.
    out8(PIC2_DATA, PIC_IRQ_VECTOR_OFFSET + 8);
    io_wait();
    
    // TELL MASTER THERE IS A SLAVE ON IRQ2.
    out8(PIC1_DATA, 4);
    io_wait();
    
    // TELL SLAVE IT'S CASCADE IDENTITY.
    out8(PIC2_DATA, 2);
    io_wait();
    
    // NOT QUITE SURE WHAT THIS DOES YET.
    out8(PIC1_DATA, ICW4_8086);
    io_wait();
    
    out8(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // LOAD INTERRUPT MASKS.
    out8(PIC1_DATA, parentMasks);
    io_wait();
    
    out8(PIC2_DATA, childMasks);
    io_wait();
}