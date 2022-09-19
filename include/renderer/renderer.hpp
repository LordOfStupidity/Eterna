#ifndef _RENDERER_HPP
#define _RENDERER_HPP

#include <cstdint>

#include <int.hpp>
#include <math.hpp>
#include <memory/memory.hpp>

struct PSF1_HEADER {
    // Magic bytes to indicate PSF1 font type
    uint8_t Magic[2];
    uint8_t Mode;
    uint8_t CharacterSize;
};

struct PSF1_FONT {
    PSF1_HEADER* PSF1_Header;
    void* GlyphBuffer;
};

struct Framebuffer {
    void* BaseAddress;
    uint64_t BufferSize;
    uint32_t PixelWidth;
    uint32_t PixelHeight;
    uint32_t PixelsPerScanLine;

    Framebuffer() {
        BaseAddress = nullptr;
        BufferSize = 0;
        PixelWidth = 0;
        PixelHeight = 0;
        PixelsPerScanLine = 0;
    }
};

constexpr uint32_t BytesPerPixel = 4;

class Renderer {
   public:
    // Target = framebuffer to draw to in memory
    Framebuffer* Render{nullptr};
    Framebuffer* Target{nullptr};
    PSF1_FONT* Font{nullptr};
    uint32_t BackgroundColor{0x00000000};

    Renderer() {}
    Renderer(Framebuffer* render, PSF1_FONT* f);

    // Ensure draw position is within framebuffer
    // used by renderer internally to ensure valid arguments.
    void clamp_draw_position(Vector2<uint64_t>& position);

    // Update memory contents of render from target
    void swap() {
        memcpy(Target->BaseAddress, Render->BaseAddress, Target->BufferSize);
    }

    // Update size of memory contents of render from target at position
    void swap(Vector2<uint64_t> position, Vector2<uint64_t> size);

    void readpix(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                 uint32_t* buffer);

    // change every pixel in target framebuffer to background color.
    void clear() {
        uint32_t* pixel_ptr = (uint32_t*)Target->BaseAddress;
        for (uint64_t y = 0; y < Target->PixelHeight; y++) {
            for (uint64_t x = 0; x < Target->PixelWidth; x++) {
                *(uint32_t*)(pixel_ptr + x + (y * Target->PixelsPerScanLine)) =
                    BackgroundColor;
            }
        }
    }

    // update background color to given color, then clear screen
    void clear(uint32_t color) {
        BackgroundColor = color;
        clear();
    }

    void clear(Vector2<uint64_t> positon, Vector2<uint64_t> size);
    void clear(Vector2<uint64_t> positon, Vector2<uint64_t> size,
               uint32_t color);

    // Remove a single character behind position
    void clearChar(Vector2<uint64_t>& position);

    // '\r'
    void cret(Vector2<uint64_t>& position);
    // '\n'
    void newl(Vector2<uint64_t>& position);
    // '\r' + '\n'
    void crlf(Vector2<uint64_t>& position);
    void crlf(Vector2<uint64_t>& position, uint32_t offset);

    // Draw `size` of rectangle as `color`
    void drawRect(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                  uint32_t color = 0xffffffff);

    // Draw `size` of `pixels` buffer into target framebuffer
    void drawPix(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                 uint32_t* pixels);

    // Draw `size` of `bitmap` as `color`
    void drawBMP(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                 const uint8_t* bitmap, uint32_t color = 0xffffffff);

    /** 
     *  @brief Draw `size` of `bitmap` as `color`, but don't clear `0` to background color.
     *      This allows the use of bitmaps acting on alpha as well as color.
    */
    void drawBMPOver(Vector2<uint64_t>& position, Vector2<uint64_t> size,
                     const uint8_t* bitmap, uint32_t color = 0xffffffff);

    void drawChar(Vector2<uint64_t>& position, char c,
                  uint32_t color = 0xffffffff);

    void drawCharOver(Vector2<uint64_t>& position, char c,
                      uint32_t color = 0xffffffff);

    // use font to put a character to the screen
    void putChar(Vector2<uint64_t>& position, char c,
                 uint32_t color = 0xffffffff);

    // put a null-terminated string of characters to the screen, wrapping if necessary
    void puts(Vector2<uint64_t>& position, const char* str,
              uint32_t color = 0xffffffff);
};

extern Renderer gRend;

#endif  // !_RENDERER_HPP