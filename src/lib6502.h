#ifndef LIB6502_H
#define LIB6502_H

#include <stdint.h>

#define bool char
#define true 1
#define false 0

#define PC cpu->pc
#define OPCODE (FETCH(cpu, cpu->pc))

#define NF 128
#define VF 64
#define BF 16
#define DF 8
#define IF 4
#define ZF 2
#define CF 1

typedef struct {
    uint8_t A, X, Y;
    uint8_t sr;
    uint8_t sp;
    uint16_t pc;
    bool irq, nmi;
    uint8_t *memory;
} m6502;

uint8_t FETCH(m6502 *cpu, uint16_t address);
uint16_t FETCH16(m6502 *cpu, uint16_t address);
void WRITE(m6502 *cpu, uint16_t address, uint8_t value);

void init(m6502 *cpu);
void reset(m6502 *cpu);
void add_break_point(uint16_t address);
void load_rom_to_ram(m6502 *cpu, const char *filename, uint16_t ram_location);
void request_interrupt(m6502 *cpu, bool maskable);
void execute_instr(m6502 *cpu, bool debug_info);

#endif
