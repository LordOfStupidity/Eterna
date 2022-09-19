#include <cstddef>
#include <cstdint>
#include <cstr.hpp>
#include <debug.hpp>
#include <renderer/renderer.hpp>
#include <string.hpp>
#include <uart.hpp>
#include <va_list.hpp>

void dbgmsg_c(char c) {
    UART::outc(c);
}

void dbgmsg_s(const char* str) {
    UART::out(str);
}

void dbgmsg_buf(uint8_t* buffer, uint64_t byteCount) {
    UART::out(buffer, byteCount);
}

void dbgmsg(char character, ShouldNewline nl) {
    dbgmsg_c(character);

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(const String& str, ShouldNewline nl) {
    dbgmsg_buf(str.bytes(), str.length());

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(uint8_t* buffer, uint64_t byteCount, ShouldNewline nl) {
    dbgmsg_buf(buffer, byteCount);

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(bool value, ShouldNewline nl) {
    dbgmsg_s(value ? trueString : falseString);

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(double number, ShouldNewline nl) {
    UART::out(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(int64_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(int32_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(int16_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(int8_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg(uint64_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}
void dbgmsg(uint32_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}
void dbgmsg(uint16_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}
void dbgmsg(uint8_t number, ShouldNewline nl) {
    dbgmsg_s(to_string(number));

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgmsg_v(const uint8_t* fmt, va_list args) {
    const uint8_t* current = fmt;

    for (; *current != '\0'; ++current) {
#ifdef UART_HIDE_COLOR_CODES
        if (*current == '\033') {
            do {
                current++;
            } while (*current != 'm' && *current != '\0');

            if (*current == '\0')
                return;

            current++;

            if (*current == '\0')
                return;
        }
#endif  // UART_HIDE_COLOR_CODES
        if (*current == '%') {
            current++;

            switch (*current) {
                default:
                    // If char following `%` is unrecognized, pass through unchanged.
                    current--;
                    dbgmsg(static_cast<char>(*current));
                    break;

                case '\0':
                    // Handle '%' at end of format string.
                    dbgmsg(static_cast<char>('%'));
                    return;

                case 's':
                    // %s
                    current++;

                    if (*current != 'l') {
                        // Found %s -- string
                        dbgmsg(va_arg(args, const char*));
                        current--;
                        break;
                    }

                    dbgmsg(*(va_arg(args, const String*)));
                    break;

                case 'h':
                    // %h
                    current++;

                    // %h? <- ? is *current
                    if (*current != 'i' && *current != 'u' && *current != 'h') {
                        // Found %h -- nothing
                        current--;
                        break;
                    }

                    // %hi -- 16 bit signed integer
                    // %hu -- 16 bit unsigned integer
                    // %hh -- Lookahead for %hhi or %hhu
                    if (*current == 'i')
                        dbgmsg(static_cast<int16_t>(va_arg(args, int)));

                    if (*current == 'u')
                        dbgmsg(static_cast<uint16_t>(va_arg(args, int)));

                    if (*current == 'h') {
                        current++;

                        if (*current != 'i' && *current != 'u') {
                            // Found %hh -- nothing
                            current--;
                            break;
                        }

                        if (*current == 'i') {
                            // Found %hhi -- 8 bit signed integer
                            dbgmsg(static_cast<int8_t>(va_arg(args, int)));
                            break;
                        }

                        if (*current == 'u') {
                            // Found %hhu -- 8 bit unsigned integer
                            dbgmsg(static_cast<uint8_t>(va_arg(args, int)));
                            break;
                        }
                    }
                    break;

                case 'u':
                    current++;

                    if (*current != 'l') {
                        // Found %u -- native bit unsigned integer
                        dbgmsg(va_arg(args, unsigned));
                        current--;
                        break;
                    }

                    current++;

                    if (*current != 'l') {
                        // Found %ul -- 32 bit unsigned integer
                        dbgmsg(va_arg(args, uint32_t));
                        current--;
                        break;
                    }

                    // Found %ull -- 64 bit unsigned integer
                    dbgmsg(va_arg(args, uint64_t));
                    break;

                case 'd':
                case 'i':
                    current++;

                    if (*current != 'l') {
                        // Found %i -- native bit signed integer
                        dbgmsg(va_arg(args, signed));
                        current--;
                        break;
                    }

                    current++;

                    if (*current != 'l') {
                        // Found %il -- 32 bit unsigned integer
                        dbgmsg(va_arg(args, int32_t));
                        current--;
                        break;
                    }

                    // Found %ill -- 64 bit unsigned integer
                    dbgmsg(va_arg(args, int64_t));
                    break;

                case 'f':
                    dbgmsg((va_arg(args, double)));
                    break;

                case 'p':
                case 'x':
                    dbgmsg_s("0x");
                    dbgmsg_s(to_hexstring(va_arg(args, uint64_t)));
                    break;

                case 'c':
                    dbgmsg(static_cast<char>(va_arg(args, int)));
                    break;

                case 'b':
                    dbgmsg(static_cast<bool>(va_arg(args, int)));
                    break;
            }
        } else {
            dbgmsg(static_cast<char>(*current));
        }
    }
}

void dbgmsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dbgmsg_v(reinterpret_cast<const uint8_t*>(fmt), args);

    va_end(args);
}

void dbgrainbow(const String& str, ShouldNewline nl) {
    for (uint64_t i = 0; i < str.length(); ++i) {
        dbgmsg("\033[1;3%im", i % 6 + 1);
        dbgmsg_c(str[i]);
    }

    dbgmsg_s("\033[0m");

    if (nl == ShouldNewline::Yes)
        dbgmsg_s("\r\n");
}

void dbgrainbow(const char* str, ShouldNewline nl) {
    dbgrainbow(String(str), nl);
}
