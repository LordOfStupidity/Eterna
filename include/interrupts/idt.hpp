#ifndef _IDT_HPP
#define _IDT_HPP

#include <cstddef>
#include <cstdint>

// IA-32e System Descriptor Types
#define SYSTEM_DESCRIPTOR_TYPE_LDT 0b0010
#define SYSTEM_DESCRIPTOR_TYPE_TSS_AVAILABLE 0b1001
#define SYSTEM_DESCRIPTOR_TYPE_TSS_BUSY 0b1011
#define SYSTEM_DESCRIPTOR_TYPE_CALL_GATE 0b1100
#define SYSTEM_DESCRIPTOR_TYPE_INTERRUPT_GATE 0b1110
#define SYSTEM_DESCRIPTOR_TYPE_TRAP_GATE 0b1111

// Interrupt Descriptor Table Type Attributes
#define IDT_TA_RING0 0b10000000
#define IDT_TA_RING3 0b11100000
#define IDT_TA_InterruptGate \
    IDT_TA_RING0 | SYSTEM_DESCRIPTOR_TYPE_INTERRUPT_GATE
#define IDT_TA_TrapGate IDT_TA_RING0 | SYSTEM_DESCRIPTOR_TYPE_TRAP_GATE
#define IDT_TAT_UserInterruptGate \
    IDT_TA_RING3 | SYSTEM_DESCRIPTOR_TYPE_INTERRUPT_GATE
#define IDT_TA_UserTrapGate IDT_TA_RING3 | SYSTEM_DESCRIPTOR_TYPE_TRAP_GATE

/**
 * @brief Interrupt Descriptor Table Descriptor Entry
 *      Offset          -- The address of the interrupt handler function in memory.
 *      Selector        -- Selector for entry in Global Descriptor Table.
 *      IST             -- Interrupt Stack Table
 *      Type Attribute  -- Specifies how the interrupt will be handled by the CPU.
 *
 * Information taken from Intel Software Developer's Manual Volume 3A:
 *   Chapter 6.14 Exception and Interrupt Handling in 64-bit Mode:
 *   Figure 6-8 64-bit IDT Gate Descriptors
 *
 * For more info on System Descriptor Type, see:
 *   Intel Software Developer;s Manual Volume 3A:
 *   Chapter 3.5 System Descriptor Types:
 *   Table 3-2 System-Segment and Gate-Descriptor Types (IA-32e Mode column)
 * 
 */
struct IDTEntry {
    uint16_t Offset0;
    uint16_t Selector;
    /**
     * 0b00000011
     *         == IST
     *   ======   Zero
     */
    uint8_t IST;
    /**
     * 0b00000000
     *       ==== System Descriptor Type
     *      =     Zero
     *    ==      Descriptor Privilege level (DPL)
     *   =        Segment Present Flag
     */
    uint8_t TypeAttribute;
    uint16_t Offset1;
    uint32_t Offset2;
    uint32_t Ignore;

    void SetOffset(uint64_t offset);
    uint64_t GetOffset();
} __attribute__((packed));

/**
 * @brief Interrupt Descriptor Table Register
 *      Limit       -- The size of the descriptor table in bytes minus one.
 *      Offset      -- The address of the table in memory.
 */
struct IDTR {
    uint16_t Limit{0};
    uint64_t Offset{0};

    IDTR() {}
    IDTR(uint16_t limit, uint64_t offset);

    void install_handler(uint64_t handler_address, uint8_t entryOffset,
                         uint8_t typeAttribute = IDT_TA_InterruptGate,
                         uint8_t selector = 0x08);

    void flush() { asm volatile("lidt %0" ::"m"(*this)); }
} __attribute__((packed));

extern IDTR gIDT;

#endif  // !_IDT_HPP