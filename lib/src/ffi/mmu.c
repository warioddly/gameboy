#include "mmu.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static uint8_t rom[0x8000];      // 32 КБ ROM
static uint8_t vram[0x2000];     // 8 КБ Video RAM
static uint8_t eram[0x2000];     // 8 КБ External RAM (на картридже)
static uint8_t wram[0x2000];     // 8 КБ Work RAM
static uint8_t oam[0xA0];        // 160 Б Sprite RAM
static uint8_t hram[0x7F];       // 127 Б High RAM
static uint8_t io[0x80];         // I/O регистры
static uint8_t interrupt_enable; // IE-регистер (0xFFFF)

void memory_init() {
    memset(rom, 0, sizeof(rom));
    memset(vram, 0, sizeof(vram));
    memset(eram, 0, sizeof(eram));
    memset(wram, 0, sizeof(wram));
    memset(oam, 0, sizeof(oam));
    memset(hram, 0, sizeof(hram));
    memset(io, 0, sizeof(io));
    interrupt_enable = 0;
}

uint8_t memory_read(uint16_t address) {
    if (address <= 0x7FFF) {
        return rom[address]; // ROM
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        return vram[address - 0x8000];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        return eram[address - 0xA000];
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        return wram[address - 0xC000];
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        return wram[address - 0xE000]; // echo
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        return oam[address - 0xFE00];
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        return io[address - 0xFF00];
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        return hram[address - 0xFF80];
    } else if (address == 0xFFFF) {
        return interrupt_enable;
    }

    return 0xFF;
}

void memory_write(uint16_t address, uint8_t value) {
    if (address <= 0x7FFF) {
        return;
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        vram[address - 0x8000] = value;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        eram[address - 0xA000] = value;
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        wram[address - 0xC000] = value;
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        wram[address - 0xE000] = value; // echo
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        oam[address - 0xFE00] = value;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        io[address - 0xFF00] = value;
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        hram[address - 0xFF80] = value;
    } else if (address == 0xFFFF) {
        interrupt_enable = value;
    }
}

void memory_load_rom(const uint8_t* data, size_t size) {
    if (size > sizeof(rom)) {
        printf("ROM слишком большой! Обрезаем до 32 КБ\n");
        size = sizeof(rom);
    }
    memcpy(rom, data, size);
}