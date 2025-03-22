#include "mmu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


void mmu_init(MMU* mmu) {
    memset(mmu, 0, sizeof(MMU));
}


uint8_t mmu_read8(MMU* mmu, uint16_t addr) {

    if (!mmu->boot_completed && addr < 0x100) {
        return mmu->boot_rom[addr];
    }

    if (addr <= 0x3FFF)
        return mmu->rom[addr];
    else if (addr <= 0x7FFF)
        return mmu->rom[addr]; // без MBC
    else if (addr <= 0x9FFF)
        return mmu->vram[addr - 0x8000];
    else if (addr <= 0xBFFF)
        return mmu->eram[addr - 0xA000];
    else if (addr <= 0xDFFF)
        return mmu->wram[addr - 0xC000];
    else if (addr <= 0xFDFF)
        return mmu->wram[addr - 0xE000]; // echo
    else if (addr <= 0xFE9F)
        return mmu->oam[addr - 0xFE00];
    else if (addr <= 0xFEFF)
        return 0xFF; // недоступно
    else if (addr <= 0xFF7F)
        return mmu->io[addr - 0xFF00];
    else if (addr <= 0xFFFE)
        return mmu->hram[addr - 0xFF80];
    else if (addr == 0xFFFF)
        return mmu->ie;

    return 0xFF;
}


void mmu_write8(MMU* mmu, uint16_t addr, uint8_t val) {

    if (addr == 0xFF50 && val == 1) {
        mmu->boot_completed = true;
        return;
    }

    if (addr <= 0x7FFF) {
        // ROM - нельзя писать (позже MBC)
        // TODO: MBC
    } else if (addr <= 0x9FFF)
        mmu->vram[addr - 0x8000] = val;
    else if (addr <= 0xBFFF)
        mmu->eram[addr - 0xA000] = val;
    else if (addr <= 0xDFFF)
        mmu->wram[addr - 0xC000] = val;
    else if (addr <= 0xFDFF)
        mmu->wram[addr - 0xE000] = val; // echo
    else if (addr <= 0xFE9F)
        mmu->oam[addr - 0xFE00] = val;
    else if (addr <= 0xFEFF) {
        printf("Writing to 0xFE00-0xFEFF is prohibited\n");
    } else if (addr <= 0xFF7F)
        mmu->io[addr - 0xFF00] = val;
    else if (addr <= 0xFFFE)
        mmu->hram[addr - 0xFF80] = val;
    else if (addr == 0xFFFF)
        mmu->ie = val;
}


void mmu_load_rom(MMU* mmu, const uint8_t* data, size_t size) {
    printf("ROM title: ");
    for (int i = 0x134; i < 0x144; i++) {
        if (data[i] == 0) break;
        putchar(data[i]);
    }
    printf("\n");
    printf("ROM loaded, size = %zu\n", size);
    printf("ROM[0x100..0x110] = ");
    for (int i = 0x100; i < 0x110; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    int len = size > 0x8000 ? 0x8000 : size;
    memcpy(mmu->rom, data, len);
}