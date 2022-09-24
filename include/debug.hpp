#ifndef _DEBUG_HPP
#define _DEBUG_HPP

#include <cstddef>
#include <cstdint>
#include <string.hpp>
#include <va_list.hpp>

enum class ShouldNewline { Yes = 0, No = 1 };

// Print a single character.
void dbgmsg_c(char);

// Print a C-style null-terminated string with no formatting.
void dbgmsg_s(const char* str);

// Print a number of bytes from a given buffer as characters.
void dbgmsg_buf(uint8_t* buffer, uint64_t byteCount);

// Print a single character with an optional newline.
void dbgmsg(char, ShouldNewline nl = ShouldNewline::No);

// Print a number of bytes from a given
// buffer as characters with an optional newline.
void dbgmsg(uint8_t* buffer, uint64_t byteCount, ShouldNewline nl = ShouldNewline::No);

// Print a human readable boolean value with an optional newline.
void dbgmsg(bool, ShouldNewline nl = ShouldNewline::No);

// Print the raw bytes of a string with an optional newline.
void dbgmsg(const String&, ShouldNewline nl = ShouldNewline::No);

void dbgmsg(double, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(int64_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(int32_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(int16_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(int8_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(uint64_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(uint32_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(uint16_t, ShouldNewline nl = ShouldNewline::No);
void dbgmsg(uint8_t, ShouldNewline nl = ShouldNewline::No);

/** 
 *  @brief Print a formatted string.
 *  Supported formats:
 *      %s     -- null terminated C-style string
 *      %c     -- character
 *      %b     -- boolean
 *      %hhu   -- 8 bit unsigned integer
 *      %hu    -- 16 bit unsigned integer
 *      %u     -- native bit width unsigned integer
 *      %ul    -- 32 bit unsigned integer
 *      %ull   -- 64 bit unsigned integer
 *      %hhi   -- 8 bit signed integer
 *      %hi    -- 16 bit signed integer
 *      %i     -- native bit width signed integer
 *      %il    -- 32 bit signed integer
 *      %ill   -- 64 bit signed integer
 *      %f     -- double, 2 digits of precision
 *      %x,%p  -- 16 hexadecimal-digit 64 bit unsigned integer
 *      %sl    -- Eterna `String`
*/
void dbgmsg(const char* fmt, ...);

// Print a string with lots of colors (and no formatting)! Nyan debug :^)
void dbgrainbow(const String&, ShouldNewline nl = ShouldNewline::No);

// Print a C-style null-terminated string in lots
// of colors (and no formatting)! Nyan debug :^)
void dbgrainbow(const char* str, ShouldNewline nl = ShouldNewline::No);

#endif  // !_DEBUG_HPP