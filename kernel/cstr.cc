#include <cstdint>
#include <cstr.hpp>
#include <uart.hpp>

const char to_hex_not_supported[] = "TYPE_NOT_SUPPORTED";

uint64_t strlen(const char* a) {
    uint64_t out = 0;

    while (true) {
        if (*a == '\0')
            return out + 1;

        ++a;
        ++out;
    }
}

bool strcmp(const char* a, const char* b, uint64_t length) {
    for (uint64_t i = 0; i < length; ++i) {
        if (*a != *b)
            return false;

        ++a;
        ++b;
    }

    return true;
}

const char* to_string(bool b) {
    return b ? trueString : falseString;
}

char uint_to_str_buf[TO_STRING_BUF_SZ];
char* to_string(uint64_t value) {
    // Get necessary string length.
    uint8_t length = 0;
    uint64_t val_cpy = value;

    while (val_cpy / 10 > 0) {
        val_cpy /= 10;
        length++;
    }

    // Write characters to string (in reverse order starting at length).
    uint8_t index = 0;

    while (value / 10 > 0) {
        uint8_t remainder = value % 10;
        uint_to_str_buf[length - index] = remainder + '0';
        value /= 10;
        index++;
    }

    // Loop doesn't catch first digit.
    uint8_t remainder = value % 10;
    uint_to_str_buf[length - index] = remainder + '0';

    // Null terminate the string
    uint_to_str_buf[length + 1] = 0;
    return uint_to_str_buf;
}

char* to_string(uint32_t value) {
    return to_string((uint64_t)value);
}

char* to_string(uint16_t value) {
    return to_string((uint64_t)value);
}

char* to_string(uint8_t value) {
    return to_string((uint64_t)value);
}

char int_to_str_buf[TO_STRING_BUF_SZ];
char* to_string(int64_t value) {
    // Add negation symbol before negative values.
    uint8_t isNegative = 0;

    if (value < 0) {
        isNegative = 1;
        value *= -1;
        int_to_str_buf[0] = '-';
    }

    // Get necessary string length.
    uint8_t length = 0;
    uint64_t val_cpy = value;

    while (val_cpy / 10 > 0) {
        val_cpy /= 10;
        length++;
    }

    // Write characters to string (in reverse order starting at length).
    uint8_t index = 0;

    while (value / 10 > 0) {
        uint8_t remainder = value % 10;
        int_to_str_buf[isNegative + length - index] = remainder + '0';
        value /= 10;
        index++;
    }

    // Loop doesn't catch first digit.
    uint8_t remainder = value % 10;
    int_to_str_buf[isNegative + length - index] = remainder + '0';

    // Null terminate the string
    int_to_str_buf[isNegative + length + 1] = 0;
    return int_to_str_buf;
}

char* to_string(int32_t value) {
    return to_string((int64_t)value);
}

char* to_string(int16_t value) {
    return to_string((int64_t)value);
}

char* to_string(int8_t value) {
    return to_string((int64_t)value);
}

char dbl_to_str_buf[TO_STRING_BUF_SZ_DBL];
char* to_string(double value, uint8_t decimalPlaces) {
    // Max decimal places = 20
    if (decimalPlaces > 20) {
        decimalPlaces = 20;
    }

    char* int_ptr = (char*)to_string((int64_t)value);
    char* double_ptr = dbl_to_str_buf;

    if (value < 0) {
        value *= -1;
    }

    while (*int_ptr != 0) {
        *double_ptr = *int_ptr;
        int_ptr++;
        double_ptr++;
    }

    *double_ptr = '.';
    double_ptr++;
    double newVal = value - (int32_t)value;

    for (uint8_t i = 0; i < decimalPlaces; i++) {
        newVal *= 10;
        *double_ptr = (int32_t)newVal + '0';
        newVal -= (int32_t)newVal;
        double_ptr++;
    }

    // Null termination
    *double_ptr = 0;
    return dbl_to_str_buf;
}

// Shout-out to User:Pancakes on the osdev wiki!
// https://wiki.osdev.org/User:Pancakes
const uint8_t size = 15;
const char* hexmap = "0123456789abcdef";
const char* hexmap_capital = "0123456789ABCDEF";
char hex_to_string_buf[TO_STRING_BUF_SZ];

char* to_hexstring(uint64_t value, bool capital) {
    int8_t n = (int8_t)size;
    uint8_t i{0};

    for (; n >= 0; --n) {
        uint8_t tmp = (value >> (n * 4)) & 0xf;
        
        if (capital) {
            hex_to_string_buf[i] = hexmap_capital[tmp];
        } else {
            hex_to_string_buf[i] = hexmap[tmp];
        }
        
        ++i;
    }

    hex_to_string_buf[i] = 0;

    return hex_to_string_buf;
}

char* to_hexstring(void* ptr, bool capital) {
    return to_hexstring((uint64_t)ptr, capital);
}
