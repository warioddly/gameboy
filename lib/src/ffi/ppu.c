#include "mmu.h"
#include "ppu.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

static inline uint32_t color_from_palette(uint8_t color_index) {
    switch (color_index) {
        case 0: return 0xFFFFFFFF;
        case 1: return 0xAAAAAAFF;
        case 2: return 0x555555FF;
        case 3: return 0x000000FF;
        default: return 0xFF00FFFF;
    }
}

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void ppu_init() {
    memset(framebuffer, 0, sizeof(framebuffer));
}

void ppu_step() {
    /// generate tiles
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            uint16_t tile_x = x / 8;
            uint16_t tile_y = y / 8;
            uint16_t tile_index = tile_y * 20 + tile_x;
            uint16_t tile_address = 0x8000 + tile_index * 16;
            uint8_t tile_line = (y % 8) * 2;
            uint8_t tile_lsb = memory_read(tile_address + tile_line);
            uint8_t tile_msb = memory_read(tile_address + tile_line + 1);
            uint8_t pixel_index = (x % 8);
            uint8_t pixel_mask = 1 << (7 - pixel_index);
            uint8_t color_index = ((tile_msb & pixel_mask) ? 1 : 0) | ((tile_lsb & pixel_mask) ? 2 : 0);
            framebuffer[y * SCREEN_WIDTH + x] = color_from_palette(color_index);
        }
    }
}