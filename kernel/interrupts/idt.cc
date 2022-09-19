#include <cstddef>
#include <cstdint>
#include <interrupts/idt.hpp>
#include <memory/memory.hpp>

IDTR gIDT;

IDTR::IDTR(uint16_t limit, uint64_t offset) : Limit(limit), Offset(offset) {
    // Ensure table is zeroes out before use.
    memset((void*)Offset, 0, Limit);
}

/**
 * @brief Install an interrupt handler within the Interrupt Descriptor Table.
 * @param handler_address -- Memory address of interrupt handler to install in 64-bit unsigned
 *      integer format.
 * @param entryOffset -- vector offset of interrupt that handler will be called on.
 * @param typeAttribute -- Instructs CPU on how to prepare for the interrupt handler to execute.
 * @param selector -- Global Descriptor Table Entry Offset of code segment that handler lies within.
 */
void IDTR::install_handler(uint64_t handler_address, uint8_t entryOffset,
                           uint8_t typeAttribute, uint8_t selector) {
    IDTEntry* interrupt = (IDTEntry*)(Offset + entryOffset * sizeof(IDTEntry));
    interrupt->SetOffset((uint64_t)handler_address);
    interrupt->TypeAttribute = typeAttribute;
    interrupt->Selector = selector;
}

void IDTEntry::SetOffset(uint64_t offset) {
    Offset0 = (uint16_t)(offset & 0x000000000000ffff);
    Offset1 = (uint16_t)((offset & 0x00000000ffff0000) >> 16);
    Offset2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

uint64_t IDTEntry::GetOffset() {
    uint64_t offset = 0;

    offset |= (uint64_t)Offset0;
    offset |= ((uint64_t)Offset1) << 16;
    offset |= ((uint64_t)Offset2) << 32;

    return offset;
}