#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    uint8_t rom[0x8000];     // 32 КБ ROM (без MBC пока)
    uint8_t vram[0x2000];    // 8 КБ
    uint8_t eram[0x2000];    // External RAM
    uint8_t wram[0x2000];    // 8 КБ
    uint8_t oam[0xA0];       // 160 байт спрайтов
    uint8_t io[0x80];        // IO-регистры
    uint8_t hram[0x7F];      // High RAM
    uint8_t ie;              // interrupt enable
    uint8_t boot_rom[0x100]; // Boot ROM
    bool boot_completed;
} MMU;

void mmu_init(MMU* mmu);
uint8_t mmu_read8(MMU* mmu, uint16_t addr);
void mmu_write8(MMU* mmu, uint16_t addr, uint8_t val);
void mmu_load_rom(MMU* mmu, const uint8_t* data, size_t size);

#endif