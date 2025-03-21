#ifndef PPU_H
#define PPU_H

#include <stdint.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

extern uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void ppu_init();
void ppu_step();

#endif