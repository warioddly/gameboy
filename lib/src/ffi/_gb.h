#ifndef GB_H
#define GB_H

#include <stdint.h>

void gb_init();
void gb_step_frame();
uint32_t* gb_get_framebuffer();
void gb_load_rom(const uint8_t* data, int size);
void gb_reset();

#endif