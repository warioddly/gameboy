/// For debug
#include <stdio.h>
#include "mmu.h"
#include "cpu.h"

int main() {
//    memory_init();
//
//    memory_write(0xC000, 0x43);
//    uint8_t value = memory_read(0xC000);
//
//    printf("Значение по адресу 0xC000: 0x%02X\n", value);

    memory_init();
    cpu_init();

    // LD C, 0x81
    memory_write(0x0100, 0x0E); // LD C, n
    memory_write(0x0101, 0x81);

    // CB 11 → RL C
    memory_write(0x0102, 0xCB);
    memory_write(0x0103, 0x11);

    cpu_step(); // LD C, 0x81
    cpu_step(); // CB 11

//    printf("Регистр C: 0x%02X\n", cpu.c);
//    printf("Флаги: 0x%02X\n", cpu.f);

    return 0;
}
