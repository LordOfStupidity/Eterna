#include <arch/x86_64/gdt.hpp>

GDT gdt;
GDTDescriptor gdt_Descriptor;

void setup_gdt() {
    gdt.Null = {0, 0, 0x00, 0x00};
    gdt.Ring0Code = {0, 0xffffffff, 0b10011010, 0b10110000};
    gdt.Ring0Data = {0, 0xffffffff, 0b10010010, 0b10110000};
    gdt.Ring3Code = {0, 0xffffffff, 0b11111010, 0b10110000};
    gdt.Ring3Data = {0, 0xffffffff, 0b11110010, 0b10110000};
    gdt.TSS = {{0, 0xffffffff, 0b10001001, 0b00100000}};
}