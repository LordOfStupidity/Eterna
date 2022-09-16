#ifndef _INT_H
#define _INT_H

#include <cstddef>
#include <cstdint>

// 16 bytes
struct uint128_t {
    uint64_t a;
    uint64_t b;
} __attribute__((packed));

// 32 byes
struct uint256_t {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    uint64_t d;
} __attribute__((packed));

// 64 bytes
struct uint512_t {
    uint256_t a;
    uint256_t b;
} __attribute__((packed));

// 128 bytes
struct uint1024_t {
    uint512_t a;
    uint512_t b;
} __attribute__((packed));

// 512 bytes
struct uint4096_t {
    uint1024_t a;
    uint1024_t b;
    uint1024_t c;
    uint1024_t d;
} __attribute__((packed));

// 2048 bytes
struct uint16384_t {
    uint4096_t a;
    uint4096_t b;
    uint4096_t c;
    uint4096_t d;
} __attribute__((packed));

// 8192 bytes
struct uint65536_t {
    uint16384_t a;
    uint16384_t b;
    uint16384_t c;
    uint16384_t d;
} __attribute__((packed));

// 32768 bytes
struct uint262144_t {
    uint65536_t a;
    uint65536_t b;
    uint65536_t c;
    uint65536_t d;
} __attribute__((packed));

// 131072 bytes
struct uint1048576_t {
    uint262144_t a;
    uint262144_t b;
    uint262144_t c;
    uint262144_t d;
} __attribute__((packed));

#endif  // !_INT_H