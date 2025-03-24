#include "cpu.h"
#include "mmu.h"
#include <stdio.h>

#define FLAG_Z (1 << 7)
#define FLAG_N (1 << 6)
#define FLAG_H (1 << 5)
#define FLAG_C (1 << 4)


static MMU* mmu_ref = NULL;


void cpu_connect_mmu(MMU* mmu) {
    mmu_ref = mmu;
}


uint8_t read8(uint16_t addr) {
    return mmu_read8(mmu_ref, addr);
}


void write8(uint16_t addr, uint8_t val) {
    mmu_write8(mmu_ref, addr, val);
}


void cpu_init(CPU* cpu) {
    if (cpu == NULL) {
        printf("cpu_init: cpu is NULL\n");
        return;
    }
    cpu->af = 0x01B0;
    cpu->bc = 0x0013;
    cpu->de = 0x00D8;
    cpu->hl = 0x014D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0100;
    cpu->halted = 0;
    cpu->ime = 1;
}


int cpu_step_cb(CPU* cpu, uint8_t cbop) {
    uint8_t* reg;
    uint16_t addr;
    uint8_t val;
    int is_mem = 0;

    switch (cbop & 0x07) {
        case 0: reg = &cpu->b; break;
        case 1: reg = &cpu->c; break;
        case 2: reg = &cpu->d; break;
        case 3: reg = &cpu->e; break;
        case 4: reg = &cpu->h; break;
        case 5: reg = &cpu->l; break;
        case 6:
            addr = cpu->hl;
            val = read8(addr);
            is_mem = 1;
            break;
        case 7: reg = &cpu->a; break;
    }

    uint8_t x = (cbop >> 6) & 0x03;
    uint8_t y = (cbop >> 3) & 0x07;

    if (x == 0) {
        uint8_t r = is_mem ? val : *reg;
        uint8_t res = 0;

        switch (y) {
            case 0: // RLC
                res = (r << 1) | (r >> 7);
                cpu->f = (res == 0 ? 0x80 : 0x00) | ((r >> 7) & 0x10); // Z, C
                break;
            case 1: // RRC
                res = (r >> 1) | (r << 7);
                cpu->f = (res == 0 ? 0x80 : 0x00) | ((r & 0x01) ? 0x10 : 0x00);
                break;
            case 2: { // RL
                uint8_t c = (cpu->f & 0x10) ? 1 : 0;
                cpu->f = ((r << 1 | c) == 0 ? 0x80 : 0x00) | ((r >> 7) ? 0x10 : 0x00);
                res = (r << 1) | c;
                break;
            }
            case 3: { // RR
                uint8_t c = (cpu->f & 0x10) ? 0x80 : 0x00;
                cpu->f = (((r >> 1) | c) == 0 ? 0x80 : 0x00) | ((r & 1) ? 0x10 : 0x00);
                res = (r >> 1) | c;
                break;
            }
            case 4: // SLA
                res = r << 1;
                cpu->f = (res == 0 ? 0x80 : 0x00) | ((r >> 7) ? 0x10 : 0x00);
                break;
            case 5: // SRA
                res = (r >> 1) | (r & 0x80);
                cpu->f = (res == 0 ? 0x80 : 0x00) | ((r & 0x01) ? 0x10 : 0x00);
                break;
            case 6: // SWAP
                res = ((r & 0x0F) << 4) | ((r & 0xF0) >> 4);
                cpu->f = (res == 0) ? 0x80 : 0x00;
                break;
            case 7: // SRL
                res = r >> 1;
                cpu->f = (res == 0 ? 0x80 : 0x00) | ((r & 0x01) ? 0x10 : 0x00);
                break;
        }

        if (is_mem) {
            write8(addr, res);
            return 16;
        } else {
            *reg = res;
            return 8;
        }
    }
    else if (x == 1) {
        uint8_t r = is_mem ? val : *reg;
        uint8_t bit = (r >> y) & 1;
        cpu->f = (bit == 0 ? 0xA0 : 0x20); // Z = !bit, H = 1, N = 0
        return is_mem ? 12 : 8;
    }
    else if (x == 2) {
        if (is_mem) {
            val &= ~(1 << y);
            write8(addr, val);
            return 16;
        } else {
            *reg &= ~(1 << y);
            return 8;
        }
    }
    else if (x == 3) {
        if (is_mem) {
            val |= (1 << y);
            write8(addr, val);
            return 16;
        } else {
            *reg |= (1 << y);
            return 8;
        }
    }

    return 4;
}


int cpu_step(CPU* cpu) {

    uint8_t opcode = read8(cpu->pc++);

//    printf("PC=%04X  OP=%02X  A=%02X B=%02X C=%02X F=%02X\n", cpu->pc - 1, opcode, cpu->a, cpu->b, cpu->c, cpu->f);

    switch (opcode) {
        case 0x00: return 4; // NOP
        case 0x3E: cpu->a = read8(cpu->pc++); return 8; // LD A, n
        case 0x06: cpu->b = read8(cpu->pc++); return 8; // LD B, n
        case 0x0E: cpu->c = read8(cpu->pc++); return 8;
        case 0x16: cpu->d = read8(cpu->pc++); return 8;
        case 0x1E: cpu->e = read8(cpu->pc++); return 8;
        case 0x26: cpu->h = read8(cpu->pc++); return 8;
        case 0x2E: cpu->l = read8(cpu->pc++); return 8;

        case 0x7F: cpu->a = cpu->a; return 4;
        case 0x78: cpu->a = cpu->b; return 4;
        case 0x79: cpu->a = cpu->c; return 4;
        case 0x7A: cpu->a = cpu->d; return 4;
        case 0x7B: cpu->a = cpu->e; return 4;
        case 0x7C: cpu->a = cpu->h; return 4;
        case 0x7D: cpu->a = cpu->l; return 4;
        case 0x7E: cpu->a = read8(cpu->hl); return 8;

        case 0xA8: cpu->a &= cpu->b; cpu->f = (cpu->a == 0 ? 0x80 : 0x00); return 4; // AND B
        case 0xB0: cpu->a |= cpu->b; cpu->f = (cpu->a == 0 ? 0x80 : 0x00); return 4; // OR B
        case 0xAF: cpu->a = 0; cpu->f = 0x80; return 4; // XOR A
        case 0xFE: { // CP n
            uint8_t val = read8(cpu->pc++);
            uint8_t res = cpu->a - val;
            cpu->f = 0x40; // N
            if (res == 0) cpu->f |= 0x80; // Z
            if ((val & 0x0F) > (cpu->a & 0x0F)) cpu->f |= 0x20; // H
            if (val > cpu->a) cpu->f |= 0x10; // C
            return 8;
        }

        case 0xC3: { // JP nn
            uint8_t lo = read8(cpu->pc++);
            uint8_t hi = read8(cpu->pc++);
            cpu->pc = (hi << 8) | lo;
            return 16;
        }

        case 0xCD: { // CALL nn
            uint8_t lo = read8(cpu->pc++);
            uint8_t hi = read8(cpu->pc++);
            uint16_t addr = (hi << 8) | lo;
            cpu->sp -= 2;
            write8(cpu->sp, cpu->pc & 0xFF);
            write8(cpu->sp + 1, cpu->pc >> 8);
            cpu->pc = addr;
            return 24;
        }

        case 0xC9: { // RET
            uint8_t lo = read8(cpu->sp++);
            uint8_t hi = read8(cpu->sp++);
            cpu->pc = (hi << 8) | lo;
            return 16;
        }

        case 0xC6: { // ADD A, n
            uint8_t val = read8(cpu->pc++);
            uint16_t sum = cpu->a + val;
            cpu->f = 0;
            if ((sum & 0xFF) == 0) cpu->f |= 0x80;
            if ((cpu->a & 0x0F) + (val & 0x0F) > 0x0F) cpu->f |= 0x20;
            if (sum > 0xFF) cpu->f |= 0x10;
            cpu->a = sum & 0xFF;
            return 8;
        }

        case 0xCB: {
            uint8_t cb = read8(cpu->pc++);
            return cpu_step_cb(cpu, cb);
        }

        case 0x76: // HALT
            cpu->halted = 1;
            return 4;

        case 0xF3: cpu->ime = 0; return 4; // DI
        case 0xFB: cpu->ime_pending = 1; return 4;

        default:
//            printf("Unknown opcode 0x%02X at PC=0x%04X\n", opcode, cpu->pc - 1);
            return 4;
    }
}