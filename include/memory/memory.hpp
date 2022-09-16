#ifndef _MEMORY_HPP
#define _MEMORY_HPP

#include "int.hpp"

int memcmp(const void* aptr, const void* bptr, uint64_t numBytes);
void memcpy(const void* src, const void* dest, uint64_t numBytes);
void memset(const void* start, uint8_t value, uint64_t numBytes);

void volatile_read(const volatile void* ptr, volatile void* out, uint64_t length);
void volatile_write(void* data, volatile void* ptr, uint64_t length);

#endif  // !_MEMORY_HPP