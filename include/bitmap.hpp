#ifndef _BITMAP_HPP
#define _BITMAP_HPP

#include <cstdint>

#include "int.hpp"

class Bitmap {
public:
    Bitmap() {}

    Bitmap(uint64_t size, uint8_t* bufferAddress);

    void init(uint64_t size, uint8_t* bufferAddress);
    uint64_t length() { return Size; }
    void* base() { return (void*)Buffer; }

    bool get(uint64_t index);
    bool set(uint64_t index, bool value);

    bool operator[](uint64_t index);

private:
    uint64_t Size;
    uint8_t* Buffer;
};

#endif  // !_BITMAP_HPP