#include "ppu.h"
#include "mmu.h"
#include <string.h>
#include <stdio.h>


static const uint32_t DMG_PALETTE[4] = {
        0xFFFFFFFF, // white
        0xAAAAAAFF, // light gray
        0x555555FF, // dark gray
        0x000000FF  // black
};


static uint8_t get_tile_pixel(uint8_t* tile_data, int x, int y) {
    uint8_t byte1 = tile_data[y * 2];
    uint8_t byte2 = tile_data[y * 2 + 1];

    int bit = 7 - x;
    uint8_t lo = (byte1 >> bit) & 1;
    uint8_t hi = (byte2 >> bit) & 1;

    return (hi << 1) | lo;
}


void ppu_init(PPU* ppu) {
    memset(ppu->framebuffer, 0xFF, sizeof(ppu->framebuffer));
    ppu->scanline = 0;
    ppu->dots = 0;
    ppu->mode = 0;
}


void ppu_step(PPU* ppu, MMU* mmu, int cycles) {
    ppu->dots += cycles;

    if (ppu->dots >= 456) {
        ppu->dots -= 456;
        ppu->scanline++;

        mmu_write8(mmu, 0xFF44, ppu->scanline);

        if (ppu->scanline == 144) {
            ppu->mode = 1;
            // Можно триггерить прерывание
            // TODO: raise_interrupt(INT_VBLANK)
        } else if (ppu->scanline > 153) {
            ppu->scanline = 0;
            ppu->mode = 2; // OAM scan
        }

        if (ppu->scanline < 144) {
            // Только активные линии — отрисовываем
            // TODO: ppu_render_scanline()
        }
    }
}


void ppu_render_frame(PPU* ppu, MMU* mmu) {

    uint8_t scx = mmu_read8(mmu, 0xFF43);
    uint8_t scy = mmu_read8(mmu, 0xFF42);

    uint16_t tilemap_addr = 0x9800;
    uint16_t tiledata_addr = 0x8000;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int bg_x = (x + scx) & 0xFF;
            int bg_y = (y + scy) & 0xFF;

            int map_x = bg_x / 8;
            int map_y = bg_y / 8;
            int tile_index = mmu_read8(mmu, tilemap_addr + map_y * 32 + map_x);

            uint16_t tile_addr = tiledata_addr + tile_index * 16;
            uint8_t* tile = &mmu->vram[tile_addr - 0x8000];

            int pixel_x = bg_x % 8;
            int pixel_y = bg_y % 8;

            uint8_t color_index = get_tile_pixel(tile, pixel_x, pixel_y);
            uint32_t color = DMG_PALETTE[color_index];

            ppu->framebuffer[y][x] = color;
        }
    }
}