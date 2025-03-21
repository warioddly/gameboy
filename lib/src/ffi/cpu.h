#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef struct {
    union {
        struct { uint8_t f, a; };
        uint16_t af;
    };
    union {
        struct { uint8_t c, b; };
        uint16_t bc;
    };
    union {
        struct { uint8_t e, d; };
        uint16_t de;
    };
    union {
        struct { uint8_t l, h; };
        uint16_t hl;
    };

    uint16_t sp;
    uint16_t pc;
    uint8_t ime;
} CPU;

void cpu_init();
int cpu_step();
void cpu_step_cb();

#endif