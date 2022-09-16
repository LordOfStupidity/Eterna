#ifndef _MEMORY_COMMON_HPP
#define _MEMORY_COMMON_HPP

#include <cstdint>

#define KiB(x) ((uint64_t)(x) << 10)
#define MiB(x) ((uint64_t)(x) << 20)
#define GiB(x) ((uint64_t)(x) << 30)

#define TO_KiB(x) ((uint64_t)(x) >> 10)
#define TO_MiB(x) ((uint64_t)(x) >> 20)
#define TO_GiB(x) ((uint64_t)(x) >> 30)

constexpr uint64_t PAGE_SIZE = 4096;

#endif // !_MEMORY_COMMON_HPP