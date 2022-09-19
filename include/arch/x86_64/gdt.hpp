#ifndef _GDT_HPP
#define _GDT_HPP

#include <cstddef>
#include <cstdint>

struct GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;

    GDTDescriptor() {}
    GDTDescriptor(uint16_t size, uint64_t offset) : Size(size), Offset(offset) {}
} __attribute__((packed));


// see Section 3.4.5: Segment Descriptors and figure 3-8: Segment Descriptor of Intel Software Developer Manual, Volume 3-A
struct GDTEntry {
public:
    GDTEntry() {}
    GDTEntry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
        set_base(base);
        set_limit(limit);
        set_access(access);
        set_flags(flags);
    }

    uint32_t base() { return Base0 | Base1 << 16 | Base2 << 24; }

    uint32_t limit() { return Limit0 | (Limit1_flags & 0xf) << 16; }

    uint8_t access() { return AccessByte; }

    uint8_t flags() { return Limit1_flags & 0xf0; }

    // Limit is 20 bits and spread across two variables.
    // This helper funtion makes it easy to set the limit.
    void set_limit(uint32_t limit) {
        uint8_t flags = Limit1_flags & 0xf0;
        Limit0 = limit;
        Limit1_flags = (limit >> 16) & 0x0f;
        Limit1_flags |= flags;
    }

    // Base is 32 bits and spread across three variables.
    // This helper function makes it easy to set the base address.
    void set_base(uint32_t base) {
        Base0 = base;
        Base1 = base >> 16;
        Base2 = base >> 24;
    }

    void set_access(uint8_t access) { AccessByte = access; }

    void set_flags(uint8_t flags) {
        uint8_t limitNibble = Limit1_flags & 0x0f;
        Limit1_flags = flags & 0xf0;
        Limit1_flags |= limitNibble;
    }

private:
    // Limit 15:0
    uint16_t Limit0{0};
    // Base 15:0
    uint16_t Base0{0};
    // Base 23:16
    uint8_t Base1{0};
    // 0b00000000
    //          =  Accessed
    //         =   Readable/Writable
    //        =    Direction/Conforming
    //       =     Executable
    //      =      Descriptor Type
    //    ==       Descriptor Privilege Level
    //   =         Segment Present
    uint8_t AccessByte{0};
    // 0b00000000
    //       ==== Limit 19:16
    //      =     Available
    //     =      64-bit segment
    //    =       Default Operation Size
    //   =        Granularity (set means limit in 4kib pages)
    uint8_t Limit1_flags{0};
    // Base 31:24
    uint8_t Base2{0};
} __attribute__((packed));

class TSS_GDTEntry : public GDTEntry {
public:
    TSS_GDTEntry() {}
    TSS_GDTEntry(GDTEntry entry) : GDTEntry(entry) { Reserved = 0; }

    uint64_t base() { return GDTEntry::base() | (uint64_t)Base3 << 32; }

    void set_base(uint64_t base) {
        GDTEntry::set_base(base);
        Base3 = base >> 32;
    }

private:
    // Base 63: 32
    uint32_t Base3{0};
    uint32_t Reserved{0};
} __attribute__((packed));

// Global Descriptor Table
struct GDT {
    GDTEntry Null;      // 0x00
    GDTEntry Ring0Code; // 0x08
    GDTEntry Ring0Data; // 0x10
    GDTEntry Ring3Code; // 0x18
    GDTEntry Ring3Data; // 0x20
    TSS_GDTEntry TSS;   // 0x28, 0x30
} __attribute__((aligned(0x1000)));

void setup_gdt();

extern GDT gGDT;
extern GDTDescriptor gGDTD;

extern "C" void loadGDT(GDTDescriptor* gdtDescriptor);

#endif  // !_GDT_HPP