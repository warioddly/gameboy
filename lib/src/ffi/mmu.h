#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdio.h>

void memory_init();
uint8_t memory_read(uint16_t address);
void memory_write(uint16_t address, uint8_t value);
void memory_load_rom(const uint8_t* data, size_t size);

#endif