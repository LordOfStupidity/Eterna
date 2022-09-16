#include <bitmap.hpp>
#include <cstdint>
#include <int.hpp>
#include <memory/memory.hpp>

Bitmap::Bitmap(uint64_t size, uint8_t* bufferAddress) : Size(size), Buffer(bufferAddress) {
    memset(Buffer, 0, size);
}

void Bitmap::init(uint64_t size, uint8_t* bufferAddress) {
    Size = size;
    Buffer = bufferAddress;
    // initialize the buffer to all zeros (ensure known state).
    memset(Buffer, 0, size);
}

bool Bitmap::get(uint64_t index) {
    uint64_t byteIndex = index / 8;

    if (byteIndex >= Size) return false;

    uint8_t bitIndexer = 0b10000000 >> (index % 8);

    return (Buffer[byteIndex] & bitIndexer) > 0;
}

bool Bitmap::set(uint64_t index, bool value) {
    uint64_t byteIndex = index / 8;

    if (byteIndex >= Size) return false;

    uint8_t bitIndexer = 0b10000000 >> (index % 8);
    Buffer[byteIndex] &= ~bitIndexer;

    if (value) Buffer[byteIndex] |= bitIndexer;

    return true;
}

bool Bitmap::operator[](uint64_t index) { return get(index); }