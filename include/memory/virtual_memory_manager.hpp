#ifndef _VIRTUAL_MEMORY_MANAGER_HPP
#define _VIRTUAL_MEMORY_MANAGER_HPP

#include <cstddef>
#include <cstdint>
#include <memory/paging.hpp>

namespace Memory {
/**
 * @brief Map the entire physical address space, virtual kernel space, and
 *      finally flush the map to use it as the active mapping.
 */
void init_virtual(PageTable*);
void init_virtual();

enum class ShowDebug {
    Yes = 0,
    No = 1,
};

/**
 * @brief Map a virtual address to a physical address in the
 *      given page map level four.
 */
void map(PageTable*, void* virtualAddress, void* physicalAddress,
         uint64_t mappingFlags, ShowDebug d = ShowDebug::No);

/**
 * @brief Map a virtual address to a physical address in the
 *      currently active page map level four.
 */
void map(void* virtualAddress, void* physicalAddress, uint64_t mappingFlags,
         ShowDebug d = ShowDebug::No);

/**
 * @brief If a mapping is marked as present within the given page
 *      map level four, it will be marked as not present.
 */
void unmap(PageTable*, void* virtualAddress, ShowDebug d = ShowDebug::No);

/**
 * @brief If a mapping is marked as present within the given page
 *      map level four, it will be marked as not present.
 */
void unmap(void* virtualAddress, ShowDebug d = ShowDebug::No);

/**
 * @brief Load the given address into control register three to update
 *      the virtual to physical mapping the CPU is using currently.
 */
void flush_page_map(PageTable* pageMapLevelFour);

/**
 * @return the base address of an exact copy of the currently active page map.
 *
 * @note Does not map itself, or unmap physcial identity mapping.
 */
PageTable* clone_active_page_map();

/**
 * @return the base address of the currently active page map.
 */
PageTable* active_page_map();
}  // namespace Memory

#endif  // !_VIRTUAL_MEMORY_MANAGER_HPP