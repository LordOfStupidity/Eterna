#ifndef _PHYSICAL_MEMORY_MANAGER_HPP
#define _PHYSICAL_MEMORY_MANAGER_HPP

#include <bitmap.hpp>
#include <cstdint>
#include <linked_list.hpp>
#include <memory/efi_memory.hpp>

namespace Memory {
void init_physical(EFI_MEMORY_DESCRIPTOR* map, uint64_t size,
                   uint64_t entrySize);

// Returns the total amount of RAM in bytes.
uint64_t total_ram();
// Returns the amount of free RAM in bytes.
uint64_t free_ram();
// Returns the mount of used RAM in bytes.
uint64_t used_ram();

/**
 * @return the physical address of the base of a free
 *      page in memory, while locking it at the same time.
 */
void* request_page();

/**
 * @return the physical address of a contiguous region of physical
 *      memory that is guaranteed to have the next `numberOfPages`
 *      pages free, while locking all of them before returning.
 */
void* request_pages(uint64_t numberOfPages);

void lock_page(void* address);
void lock_pages(void* address, uint64_t numberOfPages);

void free_page(void* address);
void free_pages(void* address, uint64_t numberOfPages);

void print_debug();
void print_debug_kib();
void print_debug_mib();
}  // namespace Memory

#endif  // !_PHYSICAL_MEMORY_MANAGER_HPP