#ifndef _PAGING_HPP
#define _PAGING_HPP

#include <cstdarg>
#include <cstdint>

namespace Memory {
    enum class PageTableFlag : uint64_t {
        Present = 1ull << 0,
        ReadWrite = 1ull << 1,
        UserSuper = 1ull << 2,
        WriteThrough = 1ull << 3,
        CacheDisabled = 1ull << 4,
        Accessed = 1ull << 5,
        Dirty = 1ull << 6,
        LargerPages = 1ull << 7,
        Global = 1ull << 8,
        NX = 1ull << 63,
    };

    class PageMapIndexer {
    public:
        explicit PageMapIndexer(uint64_t virtualAddress) {
            virtualAddress >>= 12;
            PageIndex = virtualAddress & 0x1ff;

            virtualAddress >>= 9;
            PageTableIndex = virtualAddress & 0x1ff;

            virtualAddress >>= 9;
            PageDirectoryIndex = virtualAddress & 0x1ff;

            virtualAddress >>= 9;
            PageDirectoryPointerIndex = virtualAddress & 0x1ff;
        }

        uint64_t page_directory_pointer() { return PageDirectoryPointerIndex; }

        uint64_t page_directory() { return PageDirectoryIndex; }

        uint64_t page_table() { return PageTableIndex; }

        uint64_t page() { return PageIndex; }

    private:
        uint64_t PageDirectoryPointerIndex;
        uint64_t PageDirectoryIndex;
        uint64_t PageTableIndex;
        uint64_t PageIndex;
    };

    class PageDirectoryEntry {
    public:
        uint64_t address() { return (Value & 0x000ffffffffff000) >> 12; }

        void set_address(uint64_t addr) {
            addr &= 0x000000ffffffffff;
            Value &= 0xfff0000000000fff;
            Value |= (addr << 12);
        }

        bool flag(PageTableFlag flag) { return Value & (uint64_t)flag; }

        void set_flag(PageTableFlag flag, bool enabled) {
            uint64_t bitSelector = (uint64_t)flag;
            Value &= ~bitSelector;

            if (enabled) Value |= bitSelector;
        }

    private:
        uint64_t Value{0};
    } __attribute__((packed));

    struct PageTable {
        PageDirectoryEntry entries[512];
    } __attribute__((aligned(0x1000)));
}  // namespace Memory

#endif  // !_PAGING_HPP