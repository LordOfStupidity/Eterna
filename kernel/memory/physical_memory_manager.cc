#include <bitmap.hpp>
#include <cstdint>
#include <cstr.hpp>
#include <debug.hpp>
#include <link_definitions.hpp>
#include <memory/common.hpp>
#include <memory/efi_memory.hpp>
#include <memory/paging.hpp>
#include <memory/physical_memory_manager.hpp>
#include <memory/virtual_memory_manager.hpp>
#include <panic/panic.hpp>
// Uncomment the following directive for extra debug information output.
//#define DEBUG_PMM

namespace Memory {
    Bitmap PageMap;

    uint64_t TotalPages { 0 };
    uint64_t TotalFreePages { 0 };
    uint64_t TotalUsedPages { 0 };
    uint64_t MaxFreePagesInARow { 0 };

    uint64_t total_ram() {
        return TotalPages * PAGE_SIZE;
    }
    uint64_t free_ram() {
        return TotalFreePages * PAGE_SIZE;
    }
    uint64_t used_ram() {
        return TotalUsedPages * PAGE_SIZE;
    }

    void lock_page(void* address) {
        uint64_t index = (uint64_t)address / PAGE_SIZE;
        // Page already locked.
        if (PageMap.get(index))
            return;

        if (PageMap.set(index, true)) {
            TotalFreePages -= 1;
            TotalUsedPages += 1;
        }
    }

    void lock_pages(void* address, uint64_t numberOfPages) {
        for (uint64_t i = 0; i < numberOfPages; ++i)
            lock_page((void*)((uint64_t)address + (i * PAGE_SIZE)));
    }

    void free_page(void* address) {
        uint64_t index = (uint64_t)address / PAGE_SIZE;
        if (PageMap.get(index) == false)
            return;

        if (PageMap.set(index, false)) {
            TotalUsedPages -= 1;
            TotalFreePages += 1;
        }
    }

    void free_pages(void* address, uint64_t numberOfPages) {
#ifdef DEBUG_PMM
        dbgmsg("free_pages():\r\n"
               "  Address:     %x\r\n"
               "  # of pages:  %ull\r\n"
               "  Free before: %ull\r\n"
               , address
               , numberOfPages
               , TotalFreePages);
#endif
        for (uint64_t i = 0; i < numberOfPages; ++i)
            free_page((void*)((uint64_t)address + (i * PAGE_SIZE)));

#ifdef DEBUG_PMM
        dbgmsg("  Free after: %ull\r\n"
               "\r\n"
               , TotalFreePages);
#endif /* defined DEBUG_PMM */
    }

    uint64_t FirstFreePage { 0 };
    void* request_page() {
#ifdef DEBUG_PMM
        dbgmsg("request_page():\r\n"
               "  Free pages:            %ull\r\n"
               "  Max run of free pages: %ull\r\n"
               "\r\n"
               , TotalFreePages
               , MaxFreePagesInARow);
#endif
        for(; FirstFreePage < TotalPages; FirstFreePage++) {
            if (PageMap.get(FirstFreePage) == false) {
                void* addr = (void*)(FirstFreePage * PAGE_SIZE);
                lock_page(addr);
                FirstFreePage += 1; // Eat current page.
#ifdef DEBUG_PMM
                dbgmsg("  Successfully fulfilled memory request: %x\r\n"
                       "\r\n", addr);
#endif
                return addr;
            }
        }
        // TODO: Page swap from/to file on disk.
        panic("\033[31mRan out of memory in request_page() :^<\033[0m\r\n");
        return nullptr;
    }
    
    void* request_pages(uint64_t numberOfPages) {
        // Can't allocate nothing!
        if (numberOfPages == 0)
            return nullptr;
        // One page is easier to allocate than a run of contiguous pages.
        if (numberOfPages == 1)
            return request_page();
        // Can't allocate something larger than the amount of free memory.
        if (numberOfPages > TotalFreePages) {
            dbgmsg("request_pages(): \033[31mERROR\033[0m:: "
                   "Number of pages requested is larger than amount of pages available.");
            return nullptr;
        }
        if (numberOfPages > MaxFreePagesInARow) {
            dbgmsg("request_pages(): \033[31mERROR\033[0m:: "
                   "Number of pages requested is larger than any contiguous run of pages available."
                   );
            return nullptr;
        }
        
#ifdef DEBUG_PMM
        dbgmsg("request_pages():\r\n"
               "  # of pages requested:  %ull\r\n"
               "  Free pages:            %ull\r\n"
               "  Max run of free pages: %ull\r\n"
               "\r\n"
               , numberOfPages
               , TotalFreePages
               , MaxFreePagesInARow);
#endif

        for (uint64_t i = FirstFreePage; i < PageMap.length(); ++i) {
            // Skip locked pages.
            if (PageMap[i] == true)
                continue;

            // If page is free, check if entire `numberOfPages` run is free.
            uint64_t index = i;
            uint64_t run = 0;

            while (PageMap[index] == false) {
                run++;
                index++;

                if (index >= PageMap.length()) {
                    // TODO: No memory matching criteria, should
                    //   probably do a page swap from disk or something.
                    panic("\033[0mRan out of memory in request_pages() :^<\033[0m\r\n");
                    return nullptr;
                }
                if (run >= numberOfPages) {
                    void* out = (void*)(i * PAGE_SIZE);
                    lock_pages(out, numberOfPages);
#ifdef DEBUG_PMM
                    dbgmsg("  Successfully fulfilled memory request: %x\r\n"
                           "\r\n", out);
#endif
                    return out;
                }
            }
            // If this point is reached, it means run was not long enough.
            // Start searching for next run after the run we've already determined is not long enough.
            i = index;
        }
        return nullptr;
    }

    constexpr uint64_t InitialPageBitmapMaxAddress = MiB(64);
    constexpr uint64_t InitialPageBitmapPageCount = InitialPageBitmapMaxAddress / PAGE_SIZE;
    constexpr uint64_t InitialPageBitmapSize = InitialPageBitmapPageCount / 8;
    uint8_t InitialPageBitmap[InitialPageBitmapSize];

    void init_physical(EFI_MEMORY_DESCRIPTOR* memMap, uint64_t size, uint64_t entrySize) {
#ifdef DEBUG_PMM
        dbgmsg("Attempting to initialize physical memory\r\n"
               "Searching for largest free contiguous memory region under %x\r\n"
               , InitialPageBitmapMaxAddress);
#endif /* defined DEBUG_PMM */
        // Calculate number of entries within memoryMap array.
        uint64_t entries = size / entrySize;
        // Find largest free and usable contiguous region of memory
        // within space addressable by initial page bitmap.
        void* largestFreeMemorySegment { nullptr };
        uint64_t largestFreeMemorySegmentPageCount { 0 };
        for (uint64_t i = 0; i < entries; ++i) {
            EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memMap + (i * entrySize));
            if (desc->Type == 7) {
                if (desc->NumPages > largestFreeMemorySegmentPageCount
                    && (uint64_t)desc->PhysicalAddress + desc->NumPages < InitialPageBitmapMaxAddress)
                {
                    largestFreeMemorySegment = desc->PhysicalAddress;
                    largestFreeMemorySegmentPageCount = desc->NumPages;
                }
            }
            TotalPages += desc->NumPages;
        }
        if (largestFreeMemorySegment == nullptr
            || largestFreeMemorySegmentPageCount == 0)
        {
            dbgmsg("\033[31mERROR:\033[0m "
                   "Could not find free memory segment during "
                   "physical memory manager intialization."
                   );
            while (true)
                asm ("hlt");
        }
#ifdef DEBUG_PMM
        dbgmsg("Found initial free memory segment (%ullKiB) at %x\r\n"
               , TO_KiB(largestFreeMemorySegmentPageCount * PAGE_SIZE)
               , largestFreeMemorySegment
               );
#endif /* defined DEBUG_PMM */
        // Use pre-allocated memory region for initial physical page bitmap.
        PageMap.init(InitialPageBitmapSize, (uint8_t*)&InitialPageBitmap[0]);
        // Lock all pages in initial bitmap.
        lock_pages(0, InitialPageBitmapPageCount);
        // Unlock free pages in bitmap.
        for (uint64_t i = 0; i < entries; ++i) {
            EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memMap + (i * entrySize));
            if (desc->Type == 7)
                free_pages(desc->PhysicalAddress, desc->NumPages);
        }
        // Lock the kernel (in case it was just freed).
        uint64_t kernelByteCount = (uint64_t)&KERNEL_END - (uint64_t)&KERNEL_START;
        uint64_t kernelPageCount = kernelByteCount / PAGE_SIZE;
        lock_pages(&KERNEL_PHYSICAL, kernelPageCount);
        // Use the initial pre-allocated page bitmap as a guide
        // for where to allocate new virtual memory map entries.
        // Map up to the entire amount of physical memory
        // present or the max amount addressable given the
        // size limitation of the pre-allocated bitmap.
        // TODO: `.text` + `.rodata` should be read only.
        PageTable* activePML4 = active_page_map();
        for (uint64_t t = 0;
             t < TotalPages * PAGE_SIZE
                 && t < InitialPageBitmapMaxAddress;
             t += PAGE_SIZE)
        {
            map(activePML4, (void*)t, (void*)t
                , (uint64_t)PageTableFlag::Present
                | (uint64_t)PageTableFlag::ReadWrite
                | (uint64_t)PageTableFlag::Global
                );
        }
        // Calculate total number of bytes needed for a physical page
        // bitmap that covers hardware's actual amount of memory present.
        uint64_t bitmapSize = (TotalPages / 8) + 1;
        PageMap.init(bitmapSize, (uint8_t*)((uint64_t)largestFreeMemorySegment));
        TotalUsedPages = 0;
        lock_pages(0, TotalPages + 1);
        // With all pages in the bitmap locked, free only the EFI conventional memory segments.
        // We may be able to be a little more aggressive in what memory we take in the future.
        TotalFreePages = 0;
        for (uint64_t i = 0; i < entries; ++i) {
            EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memMap + (i * entrySize));
            if (desc->Type == 7) {
                free_pages(desc->PhysicalAddress, desc->NumPages);
                if (desc->NumPages > MaxFreePagesInARow)
                    MaxFreePagesInARow = desc->NumPages;
            }
        }
        /* The page map itself takes up space within the largest free memory segment.
         * As every memory segment was just set back to free in the bitmap, it's
         *   important to re-lock the page bitmap so it doesn't get trampled on
         *   when allocating more memory.
         */
        lock_pages(PageMap.base(), (PageMap.length() / PAGE_SIZE) + 1);

        // Lock the kernel in the new page bitmap (in case it already isn't).
        lock_pages(&KERNEL_PHYSICAL, kernelPageCount);

        // Calculate space that is lost due to page alignment.
        uint64_t deadSpace { 0 };
        deadSpace += (uint64_t)&DATA_START - (uint64_t)&TEXT_END;
        deadSpace += (uint64_t)&READ_ONLY_DATA_START - (uint64_t)&DATA_END;
        deadSpace += (uint64_t)&BLOCK_STARTING_SYMBOLS_START - (uint64_t)&READ_ONLY_DATA_END;
        uint64_t kernelSize = reinterpret_cast<uint64_t>(&KERNEL_END)
            - reinterpret_cast<uint64_t>(&KERNEL_START);
        uint64_t textSize = reinterpret_cast<uint64_t>(&TEXT_END)
            - reinterpret_cast<uint64_t>(&TEXT_START);
        uint64_t dataSize = reinterpret_cast<uint64_t>(&DATA_END)
            - reinterpret_cast<uint64_t>(&DATA_START);
        uint64_t rodataSize = reinterpret_cast<uint64_t>(&READ_ONLY_DATA_END)
            - reinterpret_cast<uint64_t>(&READ_ONLY_DATA_START);
        uint64_t bssSize = reinterpret_cast<uint64_t>(&BLOCK_STARTING_SYMBOLS_END)
            - reinterpret_cast<uint64_t>(&BLOCK_STARTING_SYMBOLS_START);
        dbgmsg("\033[32m"
               "Physical memory initialized"
               "\033[0m\r\n"
               "  Physical memory mapped from %x thru %x\r\n"
               "  Kernel loaded at %x (%ullMiB)\r\n"
               "  Kernel mapped from %x thru %x (%ullKiB)\r\n"
               "    .text:   %x thru %x (%ull bytes)\r\n"
               "    .data:   %x thru %x (%ull bytes)\r\n"
               "    .rodata: %x thru %x (%ull bytes)\r\n"
               "    .bss:    %x thru %x (%ull bytes)\r\n"
               "    Lost to page alignment: %ull bytes\r\n"
               "\r\n"
               , 0ULL, total_ram()
               , &KERNEL_PHYSICAL
               , TO_MiB(&KERNEL_PHYSICAL)
               , &KERNEL_START, &KERNEL_END
               , TO_KiB(kernelSize)
               , &TEXT_START, &TEXT_END
               , textSize
               , &DATA_START, &DATA_END
               , dataSize
               , &READ_ONLY_DATA_START
               , &READ_ONLY_DATA_END
               , rodataSize
               , &BLOCK_STARTING_SYMBOLS_START
               , &BLOCK_STARTING_SYMBOLS_END
               , bssSize
               , deadSpace
               );
    }

    void print_debug_kib() {
        dbgmsg("Memory Manager Debug Information:\r\n"
               "  Total Memory: %ullKiB\r\n"
               "  Free Memory: %ullKiB\r\n"
               "  Used Memory: %ullKiB\r\n"
               "\r\n"
               , TO_KiB(total_ram())
               , TO_KiB(free_ram())
               , TO_KiB(used_ram())
               );
    }

    void print_debug_mib() {
        dbgmsg("Memory Manager Debug Information:\r\n"
               "  Total Memory: %ullMiB\r\n"
               "  Free Memory: %ullMiB\r\n"
               "  Used Memory: %ullMiB\r\n"
               "\r\n"
               , TO_MiB(total_ram())
               , TO_MiB(free_ram())
               , TO_MiB(used_ram())
               );
    }

    void print_debug() {
        if (total_ram() > MiB(64))
            print_debug_mib();
        else print_debug_kib();
    }
}