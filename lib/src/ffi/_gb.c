#include "_gb.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"
#include <string.h>
#include <stdio.h>

CPU cpu;
MMU mmu;
PPU ppu;


void gb_init() {
    cpu_init(&cpu);
    mmu_init(&mmu);
    ppu_init(&ppu);

    cpu_connect_mmu(&mmu);
}


void gb_reset() {
    gb_init();
}


static void check_serial_output() {
    uint8_t sc = mmu_read8(&mmu, 0xFF02);
    if (sc == 0x81) {
        uint8_t c = mmu_read8(&mmu, 0xFF01);

        putchar(c);
        fflush(stdout);

        static char buf[256];
        static int idx = 0;
        if (idx < 255) buf[idx++] = c;

        if (c == '\n') {
            buf[idx] = '\0';
            printf(">>> %s", buf);
            idx = 0;
        }

        mmu_write8(&mmu, 0xFF02, 0x00);
    }
}


void gb_step_frame() {

    int cycles = 0;

    while (cycles < 70224 ) {  // 70224 ticks (1 frame at 60Hz)
        int step = cpu_step(&cpu);
        cycles += step;
        ppu_step(&ppu, &mmu, step);
        check_serial_output();
    }

    ppu_render_frame(&ppu, &mmu);
}


uint32_t* gb_get_framebuffer() {
    return &ppu.framebuffer[0][0];
}


void gb_load_rom(const uint8_t* data, int size) {
    mmu_load_rom(&mmu, data, size);
}