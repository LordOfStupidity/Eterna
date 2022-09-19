#ifndef _IO_HPP
#define _IO_HPP

#include <cstddef>
#include <cstdint>

void out8(uint16_t port, uint8_t value);
uint8_t in8(uint16_t port);

void out16(uint16_t port, uint16_t value);
uint16_t in16(uint16_t port);

void out32(uint16_t port, uint32_t value);
uint32_t in32(uint16_t port);

/* By writing to a port that is known to be unused,
 *   it is possible to 'delay' the CPU by a microsecond or two.
 * This is useful for 'slow' hardware (ie. RTC) that needs some
 *   time for the serial operation to take effect.
 * Port 0x80 is used by BIOS for POST codes, and reading from/writing to
 *   it is effectively guaranteed to not adversely affect the hardware.
 */
void io_wait();

#endif  // !IO_HPP