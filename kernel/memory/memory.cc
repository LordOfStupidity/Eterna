#include <int.hpp>
#include <memory/efi_memory.hpp>
#include <memory/memory.hpp>

int memcmp(void* aptr, void* bptr, uint64_t numBytes) {
    if (aptr == bptr)
        return 0;

    uint8_t* a = (uint8_t*)aptr;
    uint8_t* b = (uint8_t*)bptr;

    while (numBytes--) {
        if (*a != *b)
            return 1;
        a++;
        b++;
    }

    return 0;
}

void memcpy(const void* src, void* dest, uint64_t numBytes) {
    char* pszDest = (char*)dest;
    const char* pszSource = (const char*)src;

    if((pszDest != NULL) && (pszSource != NULL)) {
        while(numBytes) {
            // Copy byte by byte
            *(pszDest++) = *(pszSource++);
            --numBytes;
        }
    }
}

void memset(void* start, uint8_t value, uint64_t numBytes) {
    if (numBytes >= 256) {
        uint64_t qWordValue = 0;

        qWordValue |= (uint64_t)value << 0;
        qWordValue |= (uint64_t)value << 8;
        qWordValue |= (uint64_t)value << 16;
        qWordValue |= (uint64_t)value << 24;
        qWordValue |= (uint64_t)value << 32;
        qWordValue |= (uint64_t)value << 40;
        qWordValue |= (uint64_t)value << 48;
        qWordValue |= (uint64_t)value << 56;

        for (uint64_t i = 0; i <= numBytes - 8; i += 8) {
            *(uint64_t*)((uint64_t)start + i) = qWordValue;
        }
    }

    for (uint64_t i = 0; i < numBytes; ++i) {
        *(uint8_t*)((uint64_t)start + i) = value;
    }
}

// void volatile_read(const volatile void* ptr, volatile void* out,
//                    uint64_t length) {
//     if (ptr == nullptr || out == nullptr || length == 0)
//         return;

//     if (length == 1) {
//         *(uint8_t*)out = *(volatile uint8_t*)ptr;
//     } else if (length == 2) {
//         *(uint16_t*)out = *(volatile uint16_t*)ptr;
//     } else if (length == 4) {
//         *(uint32_t*)out = *(volatile uint32_t*)ptr;
//     } else if (length == 8) {
//         *(uint64_t*)out = *(volatile uint64_t*)ptr;
//     } else {
//         memcpy((void*)ptr, (void*)out, length);
//     }
// }

// void volatile_write(void* data, volatile void* ptr, uint64_t length) {
//     if (data == nullptr || ptr == nullptr || length == 0)
//         return;

//     if (length == 1) {
//         *(volatile uint8_t*)ptr = *(uint8_t*)data;
//     } else if (length == 2) {
//         *(volatile uint16_t*)ptr = *(uint16_t*)data;
//     } else if (length == 4) {
//         *(volatile uint32_t*)ptr = *(uint32_t*)data;
//     } else if (length == 8) {
//         *(volatile uint64_t*)ptr = *(uint64_t*)data;
//     } else {
//         memcpy(data, (void*)ptr, length);
//     }
// }