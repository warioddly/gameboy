#ifndef GB_H
#define GB_H

#include <stdint.h>

void gb_init();                                  // инициализация системы
void gb_step_frame();                            // один кадр эмуляции
uint32_t* gb_get_framebuffer();                  // получить указатель на framebuffer
void gb_load_rom(const uint8_t* data, int size); // загрузка ROM (из Flutter)
void gb_reset();

#endif