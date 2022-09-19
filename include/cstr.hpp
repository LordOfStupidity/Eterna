#ifndef _CSTR_HPP
#define _CSTR_HPP

#include <cstddef>
#include <cstdint>

#ifndef UART_HIDE_COLOR_CODES
constexpr const char* trueString = "\033[32mTrue\033[0m";
constexpr const char* falseString = "\033[31mFalse\033[0m";
#else
constexpr const char* trueString = "True";
constexpr const char* falseString = "False";
#endif  // !UART_HIDE_COLOR_CODES

/**
 * @brief string Length
 *
 * @return the number of characters including null terminator.
 */
uint64_t strlen(const char* a);

/**
 * @brief string Compare
 *
 * @return `true` only if all characters within both strings up
 *     both strings upto length are exactly equal.
 */
bool strcmp(const char* a, const char* b, uint64_t length);

const char* to_string(bool);

constexpr uint64_t TO_STRING_BUF_SZ = 20;
constexpr uint64_t TO_STRING_BUF_SZ_DBL = 40;

char* to_string(uint64_t);
char* to_string(uint32_t);
char* to_string(uint16_t);
char* to_string(uint8_t);

char* to_string(int64_t);
char* to_string(int32_t);
char* to_string(int16_t);
char* to_string(int8_t);

char* to_string(double, uint8_t decimalPlaces = 2);
char* to_hexstring(uint64_t value, bool capital = false);

extern const char to_hex_not_supported[];

template <typename T>
char* to_hexstring(T value, bool capital = false) {
    switch (sizeof(T)) {
        case 1:
        case 2:
        case 4:
        case 8:
            return to_hexstring((uint64_t)value, capital);

        default:
            return (char*)to_hex_not_supported;
    }
}

#endif  // !_CSTR_HPP