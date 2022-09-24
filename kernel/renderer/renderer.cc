#include <cstddef>
#include <cstdint>
#include <cstr.hpp>
#include <debug.hpp>
#include <math.hpp>
#include <memory/paging.hpp>
#include <memory/physical_memory_manager.hpp>
#include <memory/virtual_memory_manager.hpp>
#include <renderer/renderer.hpp>

// Define global renderer for use anywhere within the kernel.
Renderer gRend;

Framebuffer target;
Renderer::Renderer(Framebuffer* render, PSF1_FONT* f)
    : Render(render), Font(f) {
    /**
     * @brief Framebuffer supplied by GOP is in physical memory; map the
     *  physical memory dedicated to the framebuffer into virtual memory.
     *  Calculate size of framebuffer in pages.
     */
    uint64_t fbBase = (uint64_t)render->BaseAddress;
    uint64_t fbSize = render->BufferSize + 0x1000;
    uint64_t fbPages = fbSize / 0x1000 + 1;

    // Allocate physical pages for Render framebuffer
    Memory::lock_pages(render->BaseAddress, fbPages);

    // Map active framebuffer physical address to virtual addresses 1:1
    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 0x1000) {
        Memory::map((void*)t, (void*)t,
                    (uint64_t)Memory::PageTableFlag::Present |
                        (uint64_t)Memory::PageTableFlag::ReadWrite);
    }
    dbgmsg("  Active GOP framebuffer mapped to %x thru %x\r\n", fbBase,
           fbBase + fbSize);

    /**
     * @brief Create a new framebuffer. This memory is what will be drawn to.
     *  When the screen should be updated, this new framebuffer is copied
     *  into the active one. This helps perormance as the active framebuffer may
     *  be very slow to read/write from. Copy render framebuffer data to target.
     */
    target = *render;

    // Find physical pages for target framebuffer and allocate them.
    target.BaseAddress = Memory::request_pages(fbPages);

    if (target.BaseAddress == nullptr) {
        /**
         * @brief If memory allocation fails, pretend there are
         *  two buffer but they both point ot the same spot.
         *  This isn't the most performant, but if it works it works.
         */
        Target = Render;
    } else {
        dbgmsg("  Deferred GOP framebuffer allocated at %x thru %x\r\n",
               target.BaseAddress, (uint64_t)target.BaseAddress + fbSize);

        /**
         * @brief If memory allocation succeeds, map meory somewhere
         *  out of the way in the virtual address range. 
         */
        constexpr uint64_t virtualTargetBaseAddress = 0xffffff8000000000;
        uint64_t physicalTargetBaseAddress = (uint64_t)target.BaseAddress;

        for (uint64_t t = 0; t < fbSize; t += 0x1000) {
            Memory::map((void*)(virtualTargetBaseAddress + t),
                        (void*)(physicalTargetBaseAddress + t),
                        (uint64_t)Memory::PageTableFlag::Present |
                            (uint64_t)Memory::PageTableFlag::ReadWrite);
        }

        target.BaseAddress = (void*)virtualTargetBaseAddress;
        Target = &target;

        dbgmsg("  Deferred GOP framebuffer mapped to %x thru %x\r\n",
               virtualTargetBaseAddress, virtualTargetBaseAddress + fbSize);
    }

    clear();
    swap();
}

inline void Renderer::clamp_draw_position(Vector2<uint64_t>& position) {
    if (position.x > Target->PixelWidth)
        position.x = Target->PixelWidth;
    if (position.y > Target->PixelHeight)
        position.y = Target->PixelHeight;
}

void Renderer::swap(Vector2<uint64_t> position, Vector2<uint64_t> size) {
    if (Render->BaseAddress == Target->BaseAddress)
        return;
    // Only swap what is within the bounds of the framebuffer.
    if (position.x > Target->PixelWidth || position.y > Target->PixelHeight)
        return;
    
    // Ensure size doesn't over-run edge of framebuffer.
    uint64_t diffX = Target->PixelWidth - position.x;
    uint64_t diffY = Target->PixelHeight - position.y;
    
    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;
    
    // Calculate addresses.
    uint64_t offset =
        ((BytesPerPixel * position.x) +
         (BytesPerPixel * position.y * Target->PixelsPerScanLine));
    uint32_t* targetBaseAddress =
        (uint32_t*)((uint64_t)Target->BaseAddress + offset);
    uint32_t* renderBaseAddress =
        (uint32_t*)((uint64_t)Render->BaseAddress + offset);
    
    // Copy rectangle line-by-line.
    uint64_t bytesPerLine = BytesPerPixel * size.x;
    for (uint64_t y = 0; y < size.y; ++y) {
        memcpy(targetBaseAddress, renderBaseAddress, bytesPerLine);
        targetBaseAddress += Target->PixelsPerScanLine;
        renderBaseAddress += Render->PixelsPerScanLine;
    }
}

// carriage return ('\r')
void Renderer::cret(Vector2<uint64_t>& position) {
    position = {0, position.y};
}

// Newline('\n') or LineFeed (LF)
void Renderer::newl(Vector2<uint64_t>& position) {
    position = {position.x, position.y + Font->PSF1_Header->CharacterSize};
}

// Carriage return line feed; CRLF ('\r' + '\n')
void Renderer::crlf(Vector2<uint64_t>& position) {
    position = {0, position.y + Font->PSF1_Header->CharacterSize};
}

void Renderer::crlf(Vector2<uint64_t>& position, uint32_t offset) {
    position = {offset, position.y + Font->PSF1_Header->CharacterSize};
}

void Renderer::drawRect(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                        uint32_t color) {
    clamp_draw_position(position);

    uint32_t diffX = Target->PixelWidth - position.x;
    uint32_t diffY = Target->PixelHeight - position.y;

    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;

    uint32_t* pixel_ptr = (uint32_t*)Target->BaseAddress;

    for (uint64_t y = position.y; y < position.y + size.y; y++) {
        for (uint64_t x = position.x; x < position.x + size.x; x++) {
            *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine)) =
                color;
        }
    }
}

// Read `size` of pixel framebuffer starting at `DrawPos` into `buffer`.
void Renderer::readpix(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                       uint32_t* buffer) {
    if (buffer == nullptr)
        return;

    clamp_draw_position(position);

    uint32_t initX = size.x;
    uint32_t diffX = Target->PixelWidth - position.x;
    uint32_t diffY = Target->PixelHeight - position.y;

    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;

    uint32_t* pixel_ptr = (uint32_t*)Target->BaseAddress;

    for (uint64_t y = position.y; y < position.y + size.y; y++) {
        for (uint64_t x = position.x; x < position.x + size.x; x++) {
            *(uint32_t*)(buffer + (x - position.x) +
                         ((y - position.y) * initX)) =
                *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine));
        }
    }
}

// Draw `size` of `pixels` linear buffer starting at `DrawPos`.
void Renderer::drawPix(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                       uint32_t* pixels) {
    if (pixels == nullptr)
        return;

    clamp_draw_position(position);

    uint32_t initX = size.x;
    uint32_t diffX = Target->PixelWidth - position.x;
    uint32_t diffY = Target->PixelHeight - position.y;

    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;

    uint32_t* pixel_ptr = (uint32_t*)Target->BaseAddress;

    for (uint64_t y = position.y; y < position.y + size.y; y++) {
        for (uint64_t x = position.x; x < position.x + size.x; x++) {
            *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine)) =
                *(uint32_t*)(pixels + (x - position.x) +
                             ((y - position.y) * initX));
        }
    }
}

/** 
 *  @brief Draw `size` of a bitmap `bitmap`, using passed color `color`
 *      where bitmap is `1` and `BackgroundColor` where it is `0`.
*/
void Renderer::drawBMP(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                       const uint8_t* bitmap, uint32_t color) {
    if (bitmap == nullptr)
        return;

    clamp_draw_position(position);

    uint32_t initX = size.x;
    uint32_t diffX = Target->PixelWidth - position.x;
    uint32_t diffY = Target->PixelHeight - position.y;

    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;

    uint32_t* pixel_ptr = (uint32_t*)Target->BaseAddress;

    for (uint64_t y = position.y; y < position.y + size.y; y++) {
        for (uint64_t x = position.x; x < position.x + size.x; x++) {
            int32_t byte = ((x - position.x) + ((y - position.y) * initX)) / 8;

            if ((bitmap[byte] & (0b10000000 >> ((x - position.x) % 8))) > 0) {
                *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine)) =
                    color;
            } else {
                *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine)) =
                    BackgroundColor;
            }
        }
    }
}

// Draw `size` of a bitmap `bitmap`, using passed color `color` where bitmap is `1`.
void Renderer::drawBMPOver(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                           const uint8_t* bitmap, uint32_t color) {
    if (bitmap == nullptr)
        return;

    clamp_draw_position(position);

    uint32_t initX = size.x;
    uint32_t diffX = Target->PixelWidth - position.x;
    uint32_t diffY = Target->PixelHeight - position.y;

    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;

    uint32_t* pixel_ptr = (uint32_t*)Target->BaseAddress;

    for (uint64_t y = position.y; y < position.y + size.y; y++) {
        for (uint64_t x = position.x; x < position.x + size.x; x++) {
            int32_t byte = ((x - position.x) + ((y - position.y) * initX)) / 8;

            if ((bitmap[byte] & (0b10000000 >> ((x - position.x) % 8))) > 0)
                *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine)) =
                    color;
        }
    }
}

// Draw a character at `position` using the renderer's bitmap font.
void Renderer::drawChar(Vector2<uint64_t>& position, char c, uint32_t color) {
    // Draw character
    drawBMP(
        position, {8, Font->PSF1_Header->CharacterSize},
        (uint8_t*)Font->GlyphBuffer + (c * Font->PSF1_Header->CharacterSize),
        color);
}

/**
 * @brief Draw a character at `position` using the renderer's bitmap font,
 *      without clearing what's behind the character.
 */
void Renderer::drawCharOver(Vector2<uint64_t>& position, char c,
                            uint32_t color) {
    // Draw Character
    drawBMPOver(
        position, {8, Font->PSF1_Header->CharacterSize},
        (uint8_t*)Font->GlyphBuffer + (c * Font->PSF1_Header->CharacterSize),
        color);
}

/**
 * @brief Draw a character using the renderer's bitmap font, then increment `DrawPos`
 *      as such that another character would not overlap with the previous (ie. typing).
 */
void Renderer::putChar(Vector2<uint64_t>& position, char c, uint32_t color) {
    gRend.drawChar(position, c, color);

    // Increment pixel position horizontally by one character.
    position.x += 8;

    // Newline if next character would be off-sceen
    if (position.x + 8 > Target->PixelWidth)
        crlf(position);
}

void Renderer::clear(Vector2<uint64_t> position, Vector2<uint64_t> size) {
    // Only clear what is within the bounds of the framebuffer.
    if (position.x > Target->PixelWidth || position.y > Target->PixelHeight)
        return;

    // Ensure size doesn't over-run edge of framebuffer.
    uint64_t diffX = Target->PixelWidth - position.x;
    uint64_t diffY = Target->PixelHeight - position.y;

    if (diffX < size.x)
        size.x = diffX;
    if (diffY < size.y)
        size.y = diffY;

    // Calculate addresses.
    uint32_t* renderBaseAddress =
        (uint32_t*)((uint64_t)Render->BaseAddress +
                    (BytesPerPixel * position.x) +
                    (BytesPerPixel * position.y * Render->PixelsPerScanLine));

    // Copy rectangle line-by-line
    for (uint64_t y = 0; y < size.y; ++y) {
        for (uint64_t x = 0; x < size.x; ++x) {
            *(renderBaseAddress + x) = BackgroundColor;
        }

        renderBaseAddress += Render->PixelsPerScanLine;
    }
}

void Renderer::clear(Vector2<uint64_t> position, Vector2<uint64_t> size,
                     uint32_t color) {
    BackgroundColor = color;
    clear(position, size);
}

/**
 * @brief Clear a single character to the background color behind
 *      the position (Backspace).
 */
void Renderer::clearChar(Vector2<uint64_t>& position) {
    // Move up line if necessary.
    if (position.x < 8) {
        position.x = Target->PixelWidth;

        if (position.y >= Font->PSF1_Header->CharacterSize) {
            position.y -= Font->PSF1_Header->CharacterSize;
        } else {
            position = {8, 0};
        }
    }

    position.x -= 8;

    drawRect(position, {8, Font->PSF1_Header->CharacterSize}, BackgroundColor);
}

/**
 * @brief Put a string of characters `str` (null terminated) to the screen
 *      with color `color` at `position`.
 */
void Renderer::puts(Vector2<uint64_t>& position, const char* str,
                    uint32_t color) {
    // Set current character to first character in string.
    char* c = (char*)str;

    // Loop over string until null-terminator.
    while (*c != 0) {
        // put current character of string at current pixel position.
        putChar(position, *c, color);
        c++;
    }
}