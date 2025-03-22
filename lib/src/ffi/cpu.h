#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdlib.h>
#include "mmu.h"

typedef struct {
    union {
        struct {
            uint8_t f; // flags: Z, N, H, C
            uint8_t a;
        };
        uint16_t af;
    };
    union {
        struct {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };
    union {
        struct {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };
    union {
        struct {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };

    uint16_t sp;
    uint16_t pc;

    int halted;
    int ime; // interrupt master enable
} CPU;

void cpu_init(CPU* cpu);
int cpu_step(CPU* cpu);
int cpu_step_cb(CPU* cpu, uint8_t cbop);
void cpu_connect_mmu(MMU* mmu);

#endif