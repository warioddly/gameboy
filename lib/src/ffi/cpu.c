#include "ppu.h"
#include "cpu.h"
#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_Z 0x80 // Zero
#define FLAG_N 0x40 // Subtract
#define FLAG_H 0x20 // Half-carry
#define FLAG_C 0x10 // Carry


static CPU cpu;


static uint8_t memory_read16(uint16_t address) {
    return memory_read(address) | (memory_read(address + 1) << 8);
}


static void memory_write16(uint16_t address, uint16_t value) {
    memory_write(address, value & 0xFF);
    memory_write(address + 1, value >> 8);
}


void cpu_init() {
    memset(&cpu, 0, sizeof(CPU));
    cpu.af = 0x01B0;
    cpu.bc = 0x0013;
    cpu.de = 0x00D8;
    cpu.hl = 0x014D;
    cpu.sp = 0xFFFE;
    cpu.pc = 0x0100;
    cpu.ime = 1;
}


void cpu_step_cb() {
    int cycles = cpu_step();
    for (int i = 0; i < cycles; i++) {
        ppu_step();
    }
}


int cpu_step() {
    uint8_t opcode = memory_read(cpu.pc++);
    int cycles = 0;

    printf("PC: 0x%04X, Opcode: 0x%02X\n", cpu.pc, opcode);

    switch (opcode) {
        case 0x00: // NOP
            cycles = 4;
            break;

        case 0x01: { // LD BC, d16
            uint8_t lo = memory_read(cpu.pc++);
            uint8_t hi = memory_read(cpu.pc++);
            cpu.bc = (hi << 8) | lo;
            cycles = 12;
            break;
        }

        case 0x02: // LD (BC), A
            memory_write(cpu.bc, cpu.a);
            cycles = 8;
            break;

        case 0x03: // INC BC
            cpu.bc++;
            cycles = 8;
            break;

        case 0x04: // INC B
            cpu.b++;
            cycles = 4;
            break;

        case 0x05: // DEC B
            cpu.b--;
            cycles = 4;
            break;

        case 0x06: // LD B, d8
            cpu.b = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x07: // RLCA
            cpu.f &= 0x0F; // clear Z, N, H
            cpu.f |= (cpu.a >> 7) ? 0x10 : 0x00; // set carry if bit 7 is 1
            cpu.a = (cpu.a << 1) | (cpu.a >> 7);
            cycles = 4;
            break;

        case 0x08: { // LD (a16), SP
            uint16_t addr = memory_read(cpu.pc++);
            addr |= (memory_read(cpu.pc++) << 8);
            memory_write(addr, cpu.sp & 0xFF);
            memory_write(addr + 1, cpu.sp >> 8);
            cycles = 20;
            break;
        }

        case 0x09: // ADD HL, BC
            // HL = HL + BC
            // TODO: Update flags N, H, C
            cpu.hl += cpu.bc;
            cpu.f &= ~(1 << 6); // N = 0
            // H and C flags need proper half-carry and carry check here
            cycles = 8;
            break;

        case 0x0A: // LD A, (BC)
            cpu.a = memory_read(cpu.bc);
            cycles = 8;
            break;

        case 0x0B: // DEC BC
            cpu.bc--;
            cycles = 8;
            break;

        case 0x0C: // INC C
            cpu.c++;
            // TODO: Update flags Z, N, H
            cycles = 4;
            break;

        case 0x0D: // DEC C
            cpu.c--;
            // TODO: Update flags Z, N, H
            cycles = 4;
            break;

        case 0x0E: // LD C, d8
            cpu.c = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x0F: // RRCA
            cpu.f &= 0x0F; // Clear Z, N, H
            cpu.f |= (cpu.a & 0x01) ? 0x10 : 0x00; // Set carry if bit 0 was 1
            cpu.a = (cpu.a >> 1) | ((cpu.a & 1) << 7);
            cycles = 4;
            break;

        case 0x10: { // STOP 0
            // Обычно используется для энергосбережения. Можно проигнорировать.
            // Важно: нужно прочитать следующий байт (0), иначе нарушится последовательность.
            uint8_t next = memory_read(cpu.pc++);
            (void)next; // избегаем варнинга
            printf("STOP instruction executed. (Not fully implemented)\n");
            cycles = 4;
            break;
        }

        case 0x11: { // LD DE, d16
            uint8_t lo = memory_read(cpu.pc++);
            uint8_t hi = memory_read(cpu.pc++);
            cpu.de = (hi << 8) | lo;
            cycles = 12;
            break;
        }

        case 0x12: // LD (DE), A
            memory_write(cpu.de, cpu.a);
            cycles = 8;
            break;

        case 0x13: // INC DE
            cpu.de++;
            cycles = 8;
            break;

        case 0x14: // INC D
            cpu.d++;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x15: // DEC D
            cpu.d--;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x16: // LD D, d8
            cpu.d = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x17: { // RLA
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint8_t new_carry = (cpu.a >> 7) & 1;
            cpu.a = (cpu.a << 1) | carry;
            cpu.f = 0;
            if (new_carry) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0x18: { // JR r8
            int8_t offset = (int8_t)memory_read(cpu.pc++);
            cpu.pc += offset;
            cycles = 12;
            break;
        }

        case 0x19: // ADD HL, DE
            cpu.hl += cpu.de;
            cpu.f &= ~(1 << 6); // N = 0
            // TODO: H, C флаги
            cycles = 8;
            break;

        case 0x1A: // LD A, (DE)
            cpu.a = memory_read(cpu.de);
            cycles = 8;
            break;

        case 0x1B: // DEC DE
            cpu.de--;
            cycles = 8;
            break;

        case 0x1C: // INC E
            cpu.e++;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x1D: // DEC E
            cpu.e--;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x1E: // LD E, d8
            cpu.e = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x1F: { // RRA
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint8_t new_carry = cpu.a & 0x01;
            cpu.a = (cpu.a >> 1) | (carry << 7);
            cpu.f = 0;
            if (new_carry) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0x20: { // JR NZ, r8
            int8_t offset = (int8_t)memory_read(cpu.pc++);
            if (!(cpu.f & 0x80)) { // Z флаг сброшен
                cpu.pc += offset;
                cycles = 12;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0x21: { // LD HL, d16
            uint8_t lo = memory_read(cpu.pc++);
            uint8_t hi = memory_read(cpu.pc++);
            cpu.hl = (hi << 8) | lo;
            cycles = 12;
            break;
        }

        case 0x22: // LD (HL+), A
            memory_write(cpu.hl, cpu.a);
            cpu.hl++;
            cycles = 8;
            break;

        case 0x23: // INC HL
            cpu.hl++;
            cycles = 8;
            break;

        case 0x24: // INC H
            cpu.h++;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x25: // DEC H
            cpu.h--;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x26: // LD H, d8
            cpu.h = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x27: { // DAA
            // Decimal Adjust Accumulator — сложная инструкция, зависит от предыдущих флагов и значения A
            // Простейшая реализация (неполная, но работает для большинства случаев BCD)
            uint8_t correction = 0;
            uint8_t carry = 0;

            if ((cpu.f & 0x20) || (!(cpu.f & 0x40) && (cpu.a & 0x0F) > 9)) {
                correction |= 0x06;
            }
            if ((cpu.f & 0x10) || (!(cpu.f & 0x40) && cpu.a > 0x99)) {
                correction |= 0x60;
                carry = 0x10;
            }

            if (cpu.f & 0x40) {
                cpu.a -= correction;
            } else {
                cpu.a += correction;
            }

            cpu.f &= 0x40; // сохраняем только N
            if (cpu.a == 0) cpu.f |= 0x80;
            if (carry) cpu.f |= 0x10;

            cycles = 4;
            break;
        }

        case 0x28: { // JR Z, r8
            int8_t offset = (int8_t)memory_read(cpu.pc++);
            if (cpu.f & 0x80) { // Z флаг установлен
                cpu.pc += offset;
                cycles = 12;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0x29: // ADD HL, HL
            cpu.hl += cpu.hl;
            cpu.f &= ~(1 << 6); // N = 0
            // TODO: установить H и C
            cycles = 8;
            break;

        case 0x2A: // LD A, (HL+)
            cpu.a = memory_read(cpu.hl++);
            cycles = 8;
            break;

        case 0x2B: // DEC HL
            cpu.hl--;
            cycles = 8;
            break;

        case 0x2C: // INC L
            cpu.l++;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x2D: // DEC L
            cpu.l--;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x2E: // LD L, d8
            cpu.l = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x2F: // CPL (комплемент A — побитовая инверсия)
            cpu.a ^= 0xFF;
            cpu.f |= 0x60; // N = 1, H = 1
            cycles = 4;
            break;

        case 0x30: { // JR NC, r8
            int8_t offset = (int8_t)memory_read(cpu.pc++);
            if (!(cpu.f & 0x10)) { // C флаг сброшен
                cpu.pc += offset;
                cycles = 12;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0x31: { // LD SP, d16
            uint8_t lo = memory_read(cpu.pc++);
            uint8_t hi = memory_read(cpu.pc++);
            cpu.sp = (hi << 8) | lo;
            cycles = 12;
            break;
        }

        case 0x32: // LD (HL-), A
            memory_write(cpu.hl, cpu.a);
            cpu.hl--;
            cycles = 8;
            break;

        case 0x33: // INC SP
            cpu.sp++;
            cycles = 8;
            break;

        case 0x34: { // INC (HL)
            uint8_t val = memory_read(cpu.hl);
            val++;
            memory_write(cpu.hl, val);
            // TODO: обновить флаги Z, N, H
            cycles = 12;
            break;
        }

        case 0x35: { // DEC (HL)
            uint8_t val = memory_read(cpu.hl);
            val--;
            memory_write(cpu.hl, val);
            // TODO: обновить флаги Z, N, H
            cycles = 12;
            break;
        }

        case 0x36: { // LD (HL), d8
            uint8_t val = memory_read(cpu.pc++);
            memory_write(cpu.hl, val);
            cycles = 12;
            break;
        }

        case 0x37: // SCF
            cpu.f &= ~0x60; // сброс H и N
            cpu.f |= 0x10;  // установить C
            cycles = 4;
            break;

        case 0x38: { // JR C, r8
            int8_t offset = (int8_t)memory_read(cpu.pc++);
            if (cpu.f & 0x10) { // C флаг установлен
                cpu.pc += offset;
                cycles = 12;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0x39: // ADD HL, SP
            cpu.hl += cpu.sp;
            cpu.f &= ~(1 << 6); // N = 0
            // TODO: H, C флаги
            cycles = 8;
            break;

        case 0x3A: // LD A, (HL-)
            cpu.a = memory_read(cpu.hl--);
            cycles = 8;
            break;

        case 0x3B: // DEC SP
            cpu.sp--;
            cycles = 8;
            break;

        case 0x3C: // INC A
            cpu.a++;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x3D: // DEC A
            cpu.a--;
            // TODO: обновить флаги Z, N, H
            cycles = 4;
            break;

        case 0x3E: // LD A, d8
            cpu.a = memory_read(cpu.pc++);
            cycles = 8;
            break;

        case 0x3F: // CCF
            cpu.f &= ~0x60; // сброс H и N
            if (cpu.f & 0x10)
                cpu.f &= ~0x10; // инвертируем C
            else
                cpu.f |= 0x10;
            cycles = 4;
            break;
        case 0x40: cpu.b = cpu.b; cycles = 4; break; // LD B,B
        case 0x41: cpu.b = cpu.c; cycles = 4; break; // LD B,C
        case 0x42: cpu.b = cpu.d; cycles = 4; break; // LD B,D
        case 0x43: cpu.b = cpu.e; cycles = 4; break; // LD B,E
        case 0x44: cpu.b = cpu.h; cycles = 4; break; // LD B,H
        case 0x45: cpu.b = cpu.l; cycles = 4; break; // LD B,L
        case 0x46: cpu.b = memory_read(cpu.hl); cycles = 8; break; // LD B,(HL)
        case 0x47: cpu.b = cpu.a; cycles = 4; break; // LD B,A
        case 0x48: cpu.c = cpu.b; cycles = 4; break; // LD C,B
        case 0x49: cpu.c = cpu.c; cycles = 4; break; // LD C,C
        case 0x4A: cpu.c = cpu.d; cycles = 4; break; // LD C,D
        case 0x4B: cpu.c = cpu.e; cycles = 4; break; // LD C,E
        case 0x4C: cpu.c = cpu.h; cycles = 4; break; // LD C,H
        case 0x4D: cpu.c = cpu.l; cycles = 4; break; // LD C,L
        case 0x4E: cpu.c = memory_read(cpu.hl); cycles = 8; break; // LD C,(HL)
        case 0x4F: cpu.c = cpu.a; cycles = 4; break; // LD C,A
        case 0x50: cpu.d = cpu.b; cycles = 4; break; // LD D,B
        case 0x51: cpu.d = cpu.c; cycles = 4; break; // LD D,C
        case 0x52: cpu.d = cpu.d; cycles = 4; break; // LD D,D
        case 0x53: cpu.d = cpu.e; cycles = 4; break; // LD D,E
        case 0x54: cpu.d = cpu.h; cycles = 4; break; // LD D,H
        case 0x55: cpu.d = cpu.l; cycles = 4; break; // LD D,L
        case 0x56: cpu.d = memory_read(cpu.hl); cycles = 8; break; // LD D,(HL)
        case 0x57: cpu.d = cpu.a; cycles = 4; break; // LD D,A
        case 0x58: cpu.e = cpu.b; cycles = 4; break; // LD E,B
        case 0x59: cpu.e = cpu.c; cycles = 4; break; // LD E,C
        case 0x5A: cpu.e = cpu.d; cycles = 4; break; // LD E,D
        case 0x5B: cpu.e = cpu.e; cycles = 4; break; // LD E,E
        case 0x5C: cpu.e = cpu.h; cycles = 4; break; // LD E,H
        case 0x5D: cpu.e = cpu.l; cycles = 4; break; // LD E,L
        case 0x5E: cpu.e = memory_read(cpu.hl); cycles = 8; break; // LD E,(HL)
        case 0x5F: cpu.e = cpu.a; cycles = 4; break; // LD E,A
        case 0x60: cpu.h = cpu.b; cycles = 4; break; // LD H,B
        case 0x61: cpu.h = cpu.c; cycles = 4; break; // LD H,C
        case 0x62: cpu.h = cpu.d; cycles = 4; break; // LD H,D
        case 0x63: cpu.h = cpu.e; cycles = 4; break; // LD H,E
        case 0x64: cpu.h = cpu.h; cycles = 4; break; // LD H,H
        case 0x65: cpu.h = cpu.l; cycles = 4; break; // LD H,L
        case 0x66: cpu.h = memory_read(cpu.hl); cycles = 8; break; // LD H,(HL)
        case 0x67: cpu.h = cpu.a; cycles = 4; break; // LD H,A
        case 0x68: cpu.l = cpu.b; cycles = 4; break; // LD L,B
        case 0x69: cpu.l = cpu.c; cycles = 4; break; // LD L,C
        case 0x6A: cpu.l = cpu.d; cycles = 4; break; // LD L,D
        case 0x6B: cpu.l = cpu.e; cycles = 4; break; // LD L,E
        case 0x6C: cpu.l = cpu.h; cycles = 4; break; // LD L,H
        case 0x6D: cpu.l = cpu.l; cycles = 4; break; // LD L,L
        case 0x6E: cpu.l = memory_read(cpu.hl); cycles = 8; break; // LD L,(HL)
        case 0x6F: cpu.l = cpu.a; cycles = 4; break; // LD L,A
        case 0x70: memory_write(cpu.hl, cpu.b); cycles = 8; break; // LD (HL),B
        case 0x71: memory_write(cpu.hl, cpu.c); cycles = 8; break; // LD (HL),C
        case 0x72: memory_write(cpu.hl, cpu.d); cycles = 8; break; // LD (HL),D
        case 0x73: memory_write(cpu.hl, cpu.e); cycles = 8; break; // LD (HL),E
        case 0x74: memory_write(cpu.hl, cpu.h); cycles = 8; break; // LD (HL),H
        case 0x75: memory_write(cpu.hl, cpu.l); cycles = 8; break; // LD (HL),L
        case 0x76:
            printf("HALT executed (not fully implemented)\n");
            cycles = 4;
            break;
        case 0x77: memory_write(cpu.hl, cpu.a); cycles = 8; break; // LD (HL),A
        case 0x78: cpu.a = cpu.b; cycles = 4; break; // LD A,B
        case 0x79: cpu.a = cpu.c; cycles = 4; break; // LD A,C
        case 0x7A: cpu.a = cpu.d; cycles = 4; break; // LD A,D
        case 0x7B: cpu.a = cpu.e; cycles = 4; break; // LD A,E
        case 0x7C: cpu.a = cpu.h; cycles = 4; break; // LD A,H
        case 0x7D: cpu.a = cpu.l; cycles = 4; break; // LD A,L
        case 0x7E: cpu.a = memory_read(cpu.hl); cycles = 8; break; // LD A,(HL)
        case 0x7F: cpu.a = cpu.a; cycles = 4; break; // LD A,A
        case 0x80: { // ADD A,B
            uint16_t result = cpu.a + cpu.b;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;              // Z
            if ((cpu.a & 0x0F) + (cpu.b & 0x0F) > 0x0F) cpu.f |= 0x20; // H
            if (result > 0xFF) cpu.f |= 0x10;                     // C
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x81: { // ADD A,C
            uint16_t result = cpu.a + cpu.c;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (cpu.c & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x82: { // ADD A,D
            uint16_t result = cpu.a + cpu.d;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (cpu.d & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x83: { // ADD A,E
            uint16_t result = cpu.a + cpu.e;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (cpu.e & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x84: { // ADD A,H
            uint16_t result = cpu.a + cpu.h;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (cpu.h & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x85: { // ADD A,L
            uint16_t result = cpu.a + cpu.l;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (cpu.l & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x86: { // ADD A,(HL)
            uint8_t val = memory_read(cpu.hl);
            uint16_t result = cpu.a + val;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (val & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0x87: { // ADD A,A
            uint16_t result = cpu.a + cpu.a;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) + (cpu.a & 0x0F) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x88: { // ADC A,B
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.b + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.b & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x89: { // ADC A,C
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.c + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.c & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x8A: { // ADC A,D
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.d + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.d & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x8B: { // ADC A,E
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.e + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.e & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x8C: { // ADC A,H
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.h + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.h & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x8D: { // ADC A,L
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.l + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.l & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x8E: { // ADC A, (HL)
            uint8_t val = memory_read(cpu.hl);
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + val + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (val & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0x8F: { // ADC A,A
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + cpu.a + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (cpu.a & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x90: { // SUB B
            uint16_t result = cpu.a - cpu.b;
            cpu.f = 0x40; // N = 1
            if ((result & 0xFF) == 0) cpu.f |= 0x80; // Z
            if ((cpu.a & 0x0F) < (cpu.b & 0x0F)) cpu.f |= 0x20; // H
            if (cpu.b > cpu.a) cpu.f |= 0x10; // C
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x91: { // SUB C
            uint16_t result = cpu.a - cpu.c;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.c & 0x0F)) cpu.f |= 0x20;
            if (cpu.c > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x92: { // SUB D
            uint16_t result = cpu.a - cpu.d;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.d & 0x0F)) cpu.f |= 0x20;
            if (cpu.d > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x93: { // SUB E
            uint16_t result = cpu.a - cpu.e;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.e & 0x0F)) cpu.f |= 0x20;
            if (cpu.e > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x94: { // SUB H
            uint16_t result = cpu.a - cpu.h;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.h & 0x0F)) cpu.f |= 0x20;
            if (cpu.h > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x95: { // SUB L
            uint16_t result = cpu.a - cpu.l;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.l & 0x0F)) cpu.f |= 0x20;
            if (cpu.l > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x96: { // SUB (HL)
            uint8_t val = memory_read(cpu.hl);
            uint16_t result = cpu.a - val;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (val & 0x0F)) cpu.f |= 0x20;
            if (val > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0x97: { // SUB A
            uint16_t result = cpu.a - cpu.a;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            // Никакого half-carry или carry не будет, так как результат ноль
            cpu.a = 0;
            cycles = 4;
            break;
        }

        case 0x98: { // SBC A,B
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.b - carry;
            cpu.f = 0x40; // N = 1
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (cpu.b & 0x0F) - carry) & 0x10) cpu.f |= 0x20; // H
            if (cpu.b + carry > cpu.a) cpu.f |= 0x10; // C
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x99: { // SBC A,C
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.c - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (cpu.c & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (cpu.c + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x9A: { // SBC A,D
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.d - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (cpu.d & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (cpu.d + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x9B: { // SBC A,E
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.e - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (cpu.e & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (cpu.e + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x9C: { // SBC A,H
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.h - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (cpu.h & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (cpu.h + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x9D: { // SBC A,L
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.l - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (cpu.l & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (cpu.l + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0x9E: { // SBC A,(HL)
            uint8_t val = memory_read(cpu.hl);
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - val - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (val & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (val + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0x9F: { // SBC A,A
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - cpu.a - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((carry != 0) && ((cpu.a & 0x0F) < carry)) cpu.f |= 0x20;
            if (carry) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 4;
            break;
        }

        case 0xA0: // AND B
            cpu.a &= cpu.b;
            cpu.f = 0x20; // H = 1
            if (cpu.a == 0) cpu.f |= 0x80; // Z
            cycles = 4;
            break;

        case 0xA1: // AND C
            cpu.a &= cpu.c;
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 4;
            break;

        case 0xA2: // AND D
            cpu.a &= cpu.d;
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 4;
            break;

        case 0xA3: // AND E
            cpu.a &= cpu.e;
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 4;
            break;

        case 0xA4: // AND H
            cpu.a &= cpu.h;
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 4;
            break;

        case 0xA5: // AND L
            cpu.a &= cpu.l;
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 4;
            break;

        case 0xA6: { // AND (HL)
            uint8_t val = memory_read(cpu.hl);
            cpu.a &= val;
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 8;
            break;
        }

        case 0xA7: // AND A
            cpu.f = 0x20;
            if (cpu.a == 0) cpu.f |= 0x80;
            cycles = 4;
            break;

        case 0xA8: // XOR B
            cpu.a ^= cpu.b;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xA9: // XOR C
            cpu.a ^= cpu.c;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xAA: // XOR D
            cpu.a ^= cpu.d;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xAB: // XOR E
            cpu.a ^= cpu.e;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xAC: // XOR H
            cpu.a ^= cpu.h;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xAD: // XOR L
            cpu.a ^= cpu.l;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xAE: { // XOR (HL)
            uint8_t val = memory_read(cpu.hl);
            cpu.a ^= val;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 8;
            break;
        }

        case 0xAF: // XOR A (обнуляет A)
            cpu.a ^= cpu.a;
            cpu.f = 0x80; // A стал 0 => Z = 1
            cycles = 4;
            break;

        case 0xB0: // OR B
            cpu.a |= cpu.b;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB1: // OR C
            cpu.a |= cpu.c;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB2: // OR D
            cpu.a |= cpu.d;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB3: // OR E
            cpu.a |= cpu.e;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB4: // OR H
            cpu.a |= cpu.h;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB5: // OR L
            cpu.a |= cpu.l;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB6: { // OR (HL)
            uint8_t val = memory_read(cpu.hl);
            cpu.a |= val;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 8;
            break;
        }

        case 0xB7: // OR A
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 4;
            break;

        case 0xB8: { // CP B
            cpu.f = 0x40;
            if (cpu.a == cpu.b) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.b & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < cpu.b) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0xB9: { // CP C
            cpu.f = 0x40;
            if (cpu.a == cpu.c) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.c & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < cpu.c) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0xBA: { // CP D
            cpu.f = 0x40;
            if (cpu.a == cpu.d) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.d & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < cpu.d) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0xBB: { // CP E
            cpu.f = 0x40;
            if (cpu.a == cpu.e) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.e & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < cpu.e) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0xBC: { // CP H
            cpu.f = 0x40;
            if (cpu.a == cpu.h) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.h & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < cpu.h) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0xBD: { // CP L
            cpu.f = 0x40;
            if (cpu.a == cpu.l) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (cpu.l & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < cpu.l) cpu.f |= 0x10;
            cycles = 4;
            break;
        }

        case 0xBE: { // CP (HL)
            uint8_t val = memory_read(cpu.hl);
            cpu.f = 0x40;
            if (cpu.a == val) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (val & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < val) cpu.f |= 0x10;
            cycles = 8;
            break;
        }

        case 0xBF: { // CP A
            cpu.f = 0xC0; // Z=1, N=1, остальные 0
            cycles = 4;
            break;
        }

        case 0xC0: { // RET NZ
            if (!(cpu.f & 0x80)) { // Z == 0
                uint8_t lo = memory_read(cpu.sp++);
                uint8_t hi = memory_read(cpu.sp++);
                cpu.pc = (hi << 8) | lo;
                cycles = 20;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0xC1: { // POP BC
            uint8_t lo = memory_read(cpu.sp++);
            uint8_t hi = memory_read(cpu.sp++);
            cpu.bc = (hi << 8) | lo;
            cycles = 12;
            break;
        }

        case 0xC2: { // JP NZ, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (!(cpu.f & 0x80)) { // Z == 0
                cpu.pc = addr;
                cycles = 16;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xC3: { // JP a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            cpu.pc = addr;
            cycles = 16;
            break;
        }

        case 0xC4: { // CALL NZ, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (!(cpu.f & 0x80)) { // Z == 0
                cpu.sp -= 2;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                memory_write(cpu.sp + 1, cpu.pc >> 8);
                cpu.pc = addr;
                cycles = 24;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xC5: { // PUSH BC
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.bc & 0xFF);
            memory_write(cpu.sp + 1, cpu.bc >> 8);
            cycles = 16;
            break;
        }

        case 0xC6: { // ADD A, d8
            uint8_t val = memory_read(cpu.pc++);
            uint16_t result = cpu.a + val;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (val & 0x0F)) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0xC7: { // RST 00H
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = 0x00;
            cycles = 16;
            break;
        }

        case 0xC8: { // RET Z
            if (cpu.f & 0x80) { // Z == 1
                uint8_t lo = memory_read(cpu.sp++);
                uint8_t hi = memory_read(cpu.sp++);
                cpu.pc = (hi << 8) | lo;
                cycles = 20;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0xC9: { // RET
            uint8_t lo = memory_read(cpu.sp++);
            uint8_t hi = memory_read(cpu.sp++);
            cpu.pc = (hi << 8) | lo;
            cycles = 16;
            break;
        }

        case 0xCA: { // JP Z, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (cpu.f & 0x80) { // Z == 1
                cpu.pc = addr;
                cycles = 16;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xCB: { // CB prefix
            cpu_step_cb(); // вызываем CB-инструкцию из отдельной таблицы
            cycles = 8; // будет обновлено внутри cb-обработчика
            break;
        }

        case 0xCC: { // CALL Z, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (cpu.f & 0x80) { // Z == 1
                cpu.sp -= 2;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                memory_write(cpu.sp + 1, cpu.pc >> 8);
                cpu.pc = addr;
                cycles = 24;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xCD: { // CALL a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = addr;
            cycles = 24;
            break;
        }

        case 0xCE: { // ADC A, d8
            uint8_t val = memory_read(cpu.pc++);
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a + val + carry;
            cpu.f = 0;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) + (val & 0x0F) + carry) > 0x0F) cpu.f |= 0x20;
            if (result > 0xFF) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0xCF: { // RST 08H
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = 0x08;
            cycles = 16;
            break;
        }

        case 0xD0: { // RET NC
            if (!(cpu.f & 0x10)) { // C == 0
                uint8_t lo = memory_read(cpu.sp++);
                uint8_t hi = memory_read(cpu.sp++);
                cpu.pc = (hi << 8) | lo;
                cycles = 20;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0xD1: { // POP DE
            uint8_t lo = memory_read(cpu.sp++);
            uint8_t hi = memory_read(cpu.sp++);
            cpu.de = (hi << 8) | lo;
            cycles = 12;
            break;
        }

        case 0xD2: { // JP NC, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (!(cpu.f & 0x10)) { // C == 0
                cpu.pc = addr;
                cycles = 16;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xD3: // (undefined)
            printf("Undefined opcode: 0xD3\n");
            cycles = 4;
            break;

        case 0xD4: { // CALL NC, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (!(cpu.f & 0x10)) {
                cpu.sp -= 2;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                memory_write(cpu.sp + 1, cpu.pc >> 8);
                cpu.pc = addr;
                cycles = 24;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xD5: { // PUSH DE
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.de & 0xFF);
            memory_write(cpu.sp + 1, cpu.de >> 8);
            cycles = 16;
            break;
        }

        case 0xD6: { // SUB d8
            uint8_t val = memory_read(cpu.pc++);
            uint16_t result = cpu.a - val;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (val & 0x0F)) cpu.f |= 0x20;
            if (val > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0xD7: { // RST 10H
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = 0x10;
            cycles = 16;
            break;
        }

        case 0xD8: { // RET C
            if (cpu.f & 0x10) { // C == 1
                uint8_t lo = memory_read(cpu.sp++);
                uint8_t hi = memory_read(cpu.sp++);
                cpu.pc = (hi << 8) | lo;
                cycles = 20;
            } else {
                cycles = 8;
            }
            break;
        }

        case 0xD9: { // RETI
            uint8_t lo = memory_read(cpu.sp++);
            uint8_t hi = memory_read(cpu.sp++);
            cpu.pc = (hi << 8) | lo;
            cpu.ime = 1; // включить прерывания
            cycles = 16;
            break;
        }

        case 0xDA: { // JP C, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (cpu.f & 0x10) {
                cpu.pc = addr;
                cycles = 16;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xDB: // (undefined)
            printf("Undefined opcode: 0xDB\n");
            cycles = 4;
            break;

        case 0xDC: { // CALL C, a16
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            if (cpu.f & 0x10) {
                cpu.sp -= 2;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                memory_write(cpu.sp + 1, cpu.pc >> 8);
                cpu.pc = addr;
                cycles = 24;
            } else {
                cycles = 12;
            }
            break;
        }

        case 0xDD: // (undefined)
            printf("Undefined opcode: 0xDD\n");
            cycles = 4;
            break;

        case 0xDE: { // SBC A, d8
            uint8_t val = memory_read(cpu.pc++);
            uint8_t carry = (cpu.f & 0x10) ? 1 : 0;
            uint16_t result = cpu.a - val - carry;
            cpu.f = 0x40;
            if ((result & 0xFF) == 0) cpu.f |= 0x80;
            if (((cpu.a & 0x0F) - (val & 0x0F) - carry) & 0x10) cpu.f |= 0x20;
            if (val + carry > cpu.a) cpu.f |= 0x10;
            cpu.a = result & 0xFF;
            cycles = 8;
            break;
        }

        case 0xDF: { // RST 18H
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = 0x18;
            cycles = 16;
            break;
        }

        case 0xF0: { // LDH A, (a8)
            uint16_t addr = 0xFF00 | memory_read(cpu.pc++);
            cpu.a = memory_read(addr);
            cycles = 12;
            break;
        }

        case 0xF1: { // POP AF
            uint8_t lo = memory_read(cpu.sp++);
            uint8_t hi = memory_read(cpu.sp++);
            cpu.af = (hi << 8) | lo;
            cpu.f &= 0xF0; // нижние 4 бита флагов всегда 0
            cycles = 12;
            break;
        }

        case 0xF2: { // LD A, (C)
            uint16_t addr = 0xFF00 | cpu.c;
            cpu.a = memory_read(addr);
            cycles = 8;
            break;
        }

        case 0xF3: // DI (Disable Interrupts)
            cpu.ime = 0;
            cycles = 4;
            break;

        case 0xF4: // (undefined)
            printf("Undefined opcode: 0xF4\n");
            cycles = 4;
            break;

        case 0xF5: { // PUSH AF
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.af & 0xFF);
            memory_write(cpu.sp + 1, cpu.af >> 8);
            cycles = 16;
            break;
        }

        case 0xF6: { // OR d8
            uint8_t val = memory_read(cpu.pc++);
            cpu.a |= val;
            cpu.f = (cpu.a == 0) ? 0x80 : 0x00;
            cycles = 8;
            break;
        }

        case 0xF7: { // RST 30H
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = 0x30;
            cycles = 16;
            break;
        }

        case 0xF8: { // LD HL, SP + r8
            int8_t val = (int8_t)memory_read(cpu.pc++);
            uint16_t result = cpu.sp + val;
            cpu.hl = result;
            cpu.f = 0;
            if (((cpu.sp & 0x0F) + (val & 0x0F)) > 0x0F) cpu.f |= 0x20; // H
            if (((cpu.sp & 0xFF) + (val & 0xFF)) > 0xFF) cpu.f |= 0x10; // C
            cycles = 12;
            break;
        }

        case 0xF9: { // LD SP, HL
            cpu.sp = cpu.hl;
            cycles = 8;
            break;
        }

        case 0xFA: { // LD A, (a16)
            uint16_t addr = memory_read(cpu.pc++);
            addr |= memory_read(cpu.pc++) << 8;
            cpu.a = memory_read(addr);
            cycles = 16;
            break;
        }

        case 0xFB: // EI (Enable Interrupts)
            cpu.ime = 1;
            cycles = 4;
            break;

        case 0xFC: // (undefined)
            printf("Undefined opcode: 0xFC\n");
            cycles = 4;
            break;

        case 0xFD: // (undefined)
            printf("Undefined opcode: 0xFD\n");
            cycles = 4;
            break;

        case 0xFE: { // CP d8
            uint8_t val = memory_read(cpu.pc++);
            cpu.f = 0x40;
            if (cpu.a == val) cpu.f |= 0x80;
            if ((cpu.a & 0x0F) < (val & 0x0F)) cpu.f |= 0x20;
            if (cpu.a < val) cpu.f |= 0x10;
            cycles = 8;
            break;
        }

        case 0xFF: { // RST 38H
            cpu.sp -= 2;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            memory_write(cpu.sp + 1, cpu.pc >> 8);
            cpu.pc = 0x38;
            cycles = 16;
            break;
        }

        default:
            printf("Unimplemented opcode: 0x%02X at PC=0x%04X\n", opcode, cpu.pc - 1);
            break;
    }

    return cycles;
}
