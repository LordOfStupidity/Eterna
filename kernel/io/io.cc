#include <cstddef>
#include <cstdint>
#include <io/io.hpp>

void out8(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void out16(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void out32(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t in8(uint16_t port) {
    uint8_t retValue;
    asm volatile("inb %1, %0" : "=a"(retValue) : "Nd"(port));
    return retValue;
}

uint16_t in16(uint16_t port) {
    uint16_t retValue;
    asm volatile("inw %1, %0" : "=a"(retValue) : "Nd"(port));
    return retValue;
}

uint32_t in32(uint16_t port) {
    uint32_t retValue;
    asm volatile("inl %1, %0" : "=a"(retValue) : "Nd"(port));
    return retValue;
}

void io_wait() {
    // Port 0x80 -- Unused port that is safe to read/write
    asm volatile("outb %%al, $0x80" : : "a"(0));
}
