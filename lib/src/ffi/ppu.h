#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include "mmu.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

typedef struct {
    uint32_t framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH]; // RGBA
    int scanline;   // 0–153
    int dots;       // 0–456
    int mode;       // 0: HBlank, 1: VBlank, 2: OAM, 3: Drawing
} PPU;

void ppu_init(PPU* ppu);
void ppu_step(PPU* ppu, MMU* mmu, int cycles);
void ppu_render_frame(PPU* ppu, MMU* mmu);

#endif