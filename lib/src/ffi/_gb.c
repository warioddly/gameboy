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


static const uint16_t interrupt_vector[5] = {
        0x40, // VBlank
        0x48, // LCD STAT
        0x50, // Timer
        0x58, // Serial
        0x60  // Joypad
};


void check_interrupts() {
    uint8_t ie = mmu_read8(&mmu, 0xFFFF);   // Interrupt Enable
    uint8_t if_ = mmu_read8(&mmu, 0xFF0F);  // Interrupt Flag

    uint8_t fired = ie & if_ & 0x1F;
    if (fired == 0) return;

    if (!cpu.ime) {
        if (cpu.halted) cpu.halted = 0; // выход из HALT даже если IME = 0
        return;
    }

    for (int i = 0; i < 5; i++) {
        if (fired & (1 << i)) {
            cpu.ime = 0;
            mmu_write8(&mmu, 0xFF0F, if_ & ~(1 << i));

            cpu.sp -= 2;
            mmu_write8(&mmu,cpu.sp, cpu.pc & 0xFF);
            mmu_write8(&mmu, cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = interrupt_vector[i];

            return;
        }
    }
}


void gb_step_frame() {

    int cycles = 0;

    while (cycles < 70224 ) {  // 70224 ticks (1 frame at 60Hz)
        int step = cpu_step(&cpu);
        cycles += step;
        ppu_step(&ppu, &mmu, step);
        check_serial_output();

        if (cpu.ime_pending) {
            cpu.ime = 1;
            cpu.ime_pending = 0;
        }

        check_interrupts();
    }

    ppu_render_frame(&ppu, &mmu);
}


uint32_t* gb_get_framebuffer() {
    return &ppu.framebuffer[0][0];
}


void gb_load_rom(const uint8_t* data, int size) {
    mmu_load_rom(&mmu, data, size);
}