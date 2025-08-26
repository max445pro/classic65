#include <stdio.h>
#include <stdlib.h>
#include "lib6502.h"

typedef enum {
    M_A,
    M_ABS,
    M_ABS_X,
    M_ABS_Y,
    M_IMM,
    M_IMPL,
    M_IND,
    M_X_IND,
    M_IND_Y,
    M_REL,
    M_ZP,
    M_ZP_X,
    M_ZP_Y,
    M_NONE
} AddrMode;

static AddrMode addr_modes[256] = {
/*      0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F*/
/* 0 */ M_IMPL, M_X_IND,M_NONE, M_NONE, M_NONE, M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_A,    M_NONE, M_NONE, M_ABS,  M_ABS,  M_NONE,
/* 1 */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_NONE, M_ZP_X, M_ZP_X ,M_NONE, M_IMPL, M_ABS_Y,M_NONE, M_NONE, M_NONE, M_ABS_X,M_ABS_X,M_NONE,
/* 2 */ M_ABS,  M_X_IND,M_NONE, M_NONE, M_ZP,   M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_A,    M_NONE, M_ABS,  M_ABS,  M_ABS,  M_NONE,
/* 3 */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_NONE, M_ZP_X, M_ZP_X ,M_NONE, M_IMPL, M_ABS_Y,M_NONE, M_NONE, M_NONE, M_ABS_X,M_ABS_X,M_NONE,
/* 4 */ M_IMPL, M_X_IND,M_NONE, M_NONE, M_NONE, M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_A,    M_NONE, M_ABS,  M_ABS,  M_ABS,  M_NONE,
/* 5 */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_NONE, M_ZP_X, M_ZP_X ,M_NONE, M_IMPL, M_ABS_Y,M_NONE, M_NONE, M_NONE, M_ABS_X,M_ABS_X,M_NONE,
/* 6 */ M_IMPL, M_X_IND,M_NONE, M_NONE, M_ZP,   M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_A,    M_NONE, M_IND,  M_ABS,  M_ABS,  M_NONE,
/* 7 */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_NONE, M_ZP_X, M_ZP_X ,M_NONE, M_IMPL, M_ABS_Y,M_NONE, M_NONE, M_NONE, M_ABS_X,M_ABS_X,M_NONE,
/* 8 */ M_NONE, M_X_IND,M_NONE, M_NONE, M_ZP,   M_ZP,   M_ZP,   M_NONE, M_IMPL, M_NONE, M_IMPL, M_NONE, M_ABS,  M_ABS,  M_ABS,  M_NONE,
/* 9 */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_ZP_X, M_ZP_X, M_ZP_Y ,M_NONE, M_IMPL, M_ABS_Y,M_IMPL, M_NONE, M_NONE, M_ABS_X,M_NONE, M_NONE,
/* A */ M_IMM,  M_X_IND,M_IMM,  M_NONE, M_ZP,   M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_IMPL, M_NONE, M_ABS,  M_ABS,  M_ABS,  M_NONE,
/* B */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_ZP_X, M_ZP_X, M_ZP_Y ,M_NONE, M_IMPL, M_ABS_Y,M_IMPL, M_NONE, M_ABS_X,M_ABS_X,M_ABS_X,M_NONE,
/* C */ M_IMM,  M_X_IND,M_NONE, M_NONE, M_ZP,   M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_IMPL, M_NONE, M_ABS,  M_ABS,  M_ABS,  M_NONE,
/* D */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_NONE, M_ZP_X, M_ZP_X ,M_NONE, M_IMPL, M_ABS_Y,M_NONE, M_NONE, M_NONE, M_ABS_X,M_ABS_X,M_NONE,
/* E */ M_IMM,  M_X_IND,M_NONE, M_NONE, M_ZP,   M_ZP,   M_ZP,   M_NONE, M_IMPL, M_IMM,  M_IMPL, M_NONE, M_ABS,  M_ABS,  M_ABS,  M_NONE,
/* F */ M_REL,  M_IND_Y,M_NONE, M_NONE, M_NONE, M_ZP_X, M_ZP_X ,M_NONE, M_IMPL, M_ABS_Y,M_NONE, M_NONE, M_NONE, M_ABS_X,M_ABS_X,M_NONE,
};

/* -----Fetch & write----- */

uint8_t FETCH(m6502 *cpu, uint16_t address) {
    return cpu->memory[address];
}

uint16_t FETCH16_wrap(m6502 *cpu, uint8_t address) {
    uint8_t lo = cpu->memory[address];
    uint8_t hi = cpu->memory[(uint8_t)(address + 1)]; // zero page wrap
    return (hi << 8) | lo;
}

uint16_t FETCH16_indirectbug(m6502 *cpu, uint8_t address) {
    uint8_t lo = cpu->memory[address];
    // Bug: high byte wraps in same page
    uint8_t hi = cpu->memory[(address & 0xFF00) | ((address + 1) & 0x00FF)];
    return (hi << 8) | lo;
}

uint16_t FETCH16(m6502 *cpu, uint16_t address) {
    uint8_t lo = cpu->memory[address];
    uint8_t hi = cpu->memory[address + 1];
    return (hi << 8) | lo;
}

void WRITE(m6502 *cpu, uint16_t address, uint8_t value) {
    cpu->memory[address] = value;
}

/* -----Misc.----- */

void reset_flag_mask(m6502 *cpu, uint8_t flag_mask) {
    cpu->sr &= ~(flag_mask);
}

void set_flag_mask(m6502 *cpu, uint8_t flag_mask) {
    cpu->sr |= flag_mask;
}

void update_sign_flag(m6502 *cpu, uint8_t value) {
    if (value & 0x80) cpu->sr |= NF;
    else cpu->sr &= ~NF;
}

void update_zero_flag(m6502 *cpu, uint8_t value) {
    if (value == 0) cpu->sr |= ZF;
    else cpu->sr &= ~ZF;
}

bool check_flag_mask(m6502 *cpu, uint8_t flag) {
    return (cpu->sr & flag) != 0;
}

void push(m6502 *cpu, uint8_t value) {
    WRITE(cpu, 0x0100 + cpu->sp, value);
    cpu->sp -= 1;
}

void push16(m6502 *cpu, uint16_t value) {
    push(cpu, value & 0xff);            // Low byte
    push(cpu, (value & 0xff00) >> 8);   // High byte
}

uint8_t pull(m6502 *cpu) {
    return FETCH(cpu, ++cpu->sp);
}

uint16_t pull16(m6502 *cpu) {
    uint8_t lo = pull(cpu);
    uint8_t hi = pull(cpu);
    return lo | (hi << 8);
}

/* -----Addressing modes----- */

uint8_t *get_mode_location(m6502 *cpu, uint8_t opcode) {
    switch (addr_modes[opcode]) {
        case M_A:
            return &cpu->A;
        case M_ABS:
            return &cpu->memory[ FETCH16(cpu, PC + 1) ];
        case M_ABS_X:
            return &cpu->memory[ FETCH16(cpu, PC + 1) + cpu->X ];
        case M_ABS_Y:
            return &cpu->memory[ FETCH16(cpu, PC + 1) + cpu->Y ];
        case M_IMM:
            return &cpu->memory[ PC + 1];
        case M_IMPL:
            return NULL;
        case M_IND:
            return &cpu->memory[ FETCH16_indirectbug(cpu, FETCH16(cpu, PC + 1)) ];
        case M_X_IND:
            return &cpu->memory[ FETCH16_wrap(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->X) ) ];
        case M_IND_Y:
            return &cpu->memory[ FETCH16_wrap(cpu, FETCH(cpu, PC + 1)) + cpu->Y ];
        case M_REL:
            return NULL;    // Only needed for jump offset
        case M_ZP:
            return &cpu->memory[ FETCH(cpu, PC + 1) ];
        case M_ZP_X:
            return &cpu->memory[ (uint8_t)(FETCH(cpu, PC + 1) + cpu->X) ];
        case M_ZP_Y:
            return &cpu->memory[ (uint8_t)(FETCH(cpu, PC + 1) + cpu->Y) ];
        default:
            return NULL;
    }
}

uint8_t get_mode_value(m6502 *cpu, uint8_t opcode) {
    switch (addr_modes[opcode]) {
        case M_A:
            return cpu->A;
        case M_ABS:
            return FETCH(cpu, FETCH16(cpu, PC + 1));
        case M_ABS_X:
            return FETCH(cpu, FETCH16(cpu, PC + 1) + cpu->X);
        case M_ABS_Y:
            return FETCH(cpu, FETCH16(cpu, PC + 1) + cpu->Y);
        case M_IMM:
            return FETCH(cpu, PC + 1);
        case M_IMPL:
            return 0;
        case M_IND:
            return FETCH(cpu, FETCH16_indirectbug(cpu, FETCH16(cpu, PC + 1)));
        case M_X_IND:
            return FETCH(cpu, FETCH16_wrap(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->X)));
        case M_IND_Y:
            return FETCH(cpu, FETCH16_wrap(cpu, FETCH(cpu, PC + 1)) + cpu->Y);
        case M_REL:
            return FETCH(cpu, PC + 1);
        case M_ZP:
            return FETCH(cpu, FETCH(cpu, PC + 1));
        case M_ZP_X:
            return FETCH(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->X));
        case M_ZP_Y:
            return FETCH(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->Y));
        default:
            return 0;
    }
}

uint16_t get_jump_location(m6502 *cpu, uint8_t opcode) {
    switch (addr_modes[opcode]) {
        case M_ABS:
            return FETCH16(cpu, PC + 1);
        case M_IND:
            return FETCH16_indirectbug(cpu, FETCH16(cpu, PC + 1));
        default:
            return 0;
    }
}

void set_mode_value(m6502 *cpu, uint8_t opcode, uint8_t value) {
    switch (addr_modes[opcode]) {
        case M_A:
            cpu->A = value;
        case M_ABS:
            WRITE(cpu, FETCH16(cpu, PC + 1), value);
        case M_ABS_X:
            WRITE(cpu, FETCH16(cpu, PC + 1) + cpu->X, value);
        case M_ABS_Y:
            WRITE(cpu, FETCH16(cpu, PC + 1) + cpu->Y, value);
        case M_IMM:
            WRITE(cpu, PC + 1, value);
        case M_IMPL:
            return;
        case M_IND:
            WRITE(cpu, FETCH16_indirectbug(cpu, FETCH16(cpu, PC + 1)), value);
        case M_X_IND:
            WRITE(cpu, FETCH16_wrap(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->X) ), value);
        case M_IND_Y:
            WRITE(cpu, FETCH16_wrap(cpu, FETCH(cpu, PC + 1)) + cpu->Y, value);
        case M_REL:
            return;    // Only needed for jump offset
        case M_ZP:
            WRITE(cpu, FETCH(cpu, PC + 1), value);
        case M_ZP_X:
            WRITE(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->X), value);
        case M_ZP_Y:
            WRITE(cpu, (uint8_t)(FETCH(cpu, PC + 1) + cpu->Y), value);
        default:
            return;
    }
}

uint8_t addr_mode_size(AddrMode mode) {
    switch (mode) {
        case M_A:
        case M_IMPL:
            return 1;
        case M_IMM:
        case M_X_IND:
        case M_IND_Y:
        case M_REL:
        case M_ZP:
        case M_ZP_X:
        case M_ZP_Y:
            return 2;
        case M_ABS:
        case M_ABS_X:
        case M_ABS_Y:
        case M_IND:
            return 3;
        default:
            return 0;
    }
}

/* -----Instruction handlers----- */

/* Transfer */

void lda(m6502 *cpu) {
    cpu->A = get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void ldx(m6502 *cpu) {
    cpu->X = get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, cpu->X);
    update_sign_flag(cpu, cpu->X);
    PC += addr_mode_size(OPCODE);
}

void ldy(m6502 *cpu) {
    cpu->Y = get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, cpu->Y);
    update_sign_flag(cpu, cpu->Y);
    PC += addr_mode_size(OPCODE);
}

void sta(m6502 *cpu) {
    set_mode_value(cpu, OPCODE, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void stx(m6502 *cpu) {
    set_mode_value(cpu, OPCODE, cpu->X);
    PC += addr_mode_size(OPCODE);
}

void sty(m6502 *cpu) {
    set_mode_value(cpu, OPCODE, cpu->Y);
    PC += addr_mode_size(OPCODE);
}

void tax(m6502 *cpu) {
    cpu->X = cpu->A;
    update_zero_flag(cpu, cpu->X);
    update_sign_flag(cpu, cpu->X);
    PC += addr_mode_size(OPCODE);
}

void tay(m6502 *cpu) {
    cpu->Y = cpu->A;
    update_zero_flag(cpu, cpu->Y);
    update_sign_flag(cpu, cpu->Y);
    PC += addr_mode_size(OPCODE);
}

void tsx(m6502 *cpu) {
    cpu->X = cpu->sp;
    update_zero_flag(cpu, cpu->X);
    update_sign_flag(cpu, cpu->X);
    PC += addr_mode_size(OPCODE);
}

void txa(m6502 *cpu) {
    cpu->A = cpu->X;
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void txs(m6502 *cpu) {
    cpu->sp = cpu->X;
    PC += addr_mode_size(OPCODE);
}

void tya(m6502 *cpu) {
    cpu->A = cpu->Y;
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

/* Stack */

void php(m6502 *cpu) {
    push(cpu, cpu->sr | BF | 0b00100000);   // Push SR with break flag and bit 5 set to 1
    PC += addr_mode_size(OPCODE);
}

void pha(m6502 *cpu) {
    push(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void plp(m6502 *cpu) {
    cpu->sr = (pull(cpu) & 0b11011111) | 0b00100000;
    PC += addr_mode_size(OPCODE);
}

void pla(m6502 *cpu) {
    cpu->A = pull(cpu);
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

/* Decrements & Increments */

void dec(m6502 *cpu) {
    uint8_t result = get_mode_value(cpu, OPCODE) - 1;
    set_mode_value(cpu, OPCODE, result);
    update_zero_flag(cpu, result);
    update_sign_flag(cpu, result);
    PC += addr_mode_size(OPCODE);
}

void dex(m6502 *cpu) {
    cpu->X -= 1;
    update_zero_flag(cpu, cpu->X);
    update_sign_flag(cpu, cpu->X);
    PC += addr_mode_size(OPCODE);
}

void dey(m6502 *cpu) {
    cpu->Y -= 1;
    update_zero_flag(cpu, cpu->Y);
    update_sign_flag(cpu, cpu->Y);
    PC += addr_mode_size(OPCODE);
}

void inc(m6502 *cpu) {
    uint8_t result = get_mode_value(cpu, OPCODE) + 1;
    set_mode_value(cpu, OPCODE, result);
    update_zero_flag(cpu, result);
    update_sign_flag(cpu, result);
    PC += addr_mode_size(OPCODE);
}

void inx(m6502 *cpu) {
    cpu->X += 1;
    update_zero_flag(cpu, cpu->X);
    update_sign_flag(cpu, cpu->X);
    PC += addr_mode_size(OPCODE);
}

void iny(m6502 *cpu) {
    cpu->Y += 1;
    update_zero_flag(cpu, cpu->Y);
    update_sign_flag(cpu, cpu->Y);
    PC += addr_mode_size(OPCODE);
}

/* Arithmetic Operations */

void adc(m6502 *cpu) {
    uint8_t operand = get_mode_value(cpu, OPCODE);
    uint8_t result;

    // BCD checks
    if (check_flag_mask(cpu, DF)) {
        // Decimal mode: compute nibble sums directly
        uint16_t tmp = (cpu->A & 0x0F) + (operand & 0x0F) + check_flag_mask(cpu, CF);
        if (tmp > 9) tmp += 6;
        tmp = (tmp & 0x0F) + (cpu->A & 0xF0) + (operand & 0xF0) + (tmp > 0x0F ? 0x10 : 0);
        if (tmp > 0x9F) tmp += 0x60;

        result = (uint8_t)tmp;

        if (tmp > 0x99) set_flag_mask(cpu, CF);
        else reset_flag_mask(cpu, CF);

        // Overflow flag undefined in decimal mode (leave unchanged)
    } else {
        // Binary sum
        uint16_t sum = (uint16_t)cpu->A + operand + check_flag_mask(cpu, CF);

        // Carry flag
        if (sum > 0xff) set_flag_mask(cpu, CF);
        else reset_flag_mask(cpu, CF);

        result = (uint8_t)sum;

        // Overflow: signed overflow detection
        if (~(cpu->A ^ operand) & (cpu->A ^ result) & 0x80) set_flag_mask(cpu, VF);
        else reset_flag_mask(cpu, VF);
    }

    cpu->A = result;

    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void sbc(m6502 *cpu) {
    uint8_t operand = get_mode_value(cpu, OPCODE);
    uint8_t result;

    // BCD checks
    if (check_flag_mask(cpu, DF)) {
        // Low nibble subtraction
        int8_t low = (cpu->A & 0x0F) - (operand & 0x0F) - (!check_flag_mask(cpu, CF) ? 1 : 0);
        int8_t borrow = 0;
        if (low < 0) {
            low += 10;
            borrow = 1; // borrow to high nibble
        }

        // High nibble subtraction
        int8_t high = (cpu->A >> 4) - (operand >> 4) - borrow;
        borrow = 0;
        if (high < 0) {
            high += 10;
            borrow = 1; // final borrow
        }

        // Combine nibbles
        result = (high << 4) | (low & 0x0F);

        // Carry flag: set if no borrow occurred
        if (borrow == 0) set_flag_mask(cpu, CF);
        else reset_flag_mask(cpu, CF);

        // Overflow flag is undefined in decimal mode (leave unchanged)
    } else {
        // Binary
        uint16_t sum = (uint16_t)cpu->A + (uint8_t)(~operand) + check_flag_mask(cpu, CF);

        // Carry flag
        if (sum > 0xff) set_flag_mask(cpu, CF);
        else reset_flag_mask(cpu, CF);

        result = (uint8_t)sum;

        // Overflow: signed overflow detection
        if (~(cpu->A ^ operand) & (cpu->A ^ result) & 0x80) set_flag_mask(cpu, VF);
        else reset_flag_mask(cpu, VF);
    }

    cpu->A = result;
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

/* Logical Operations */

void ora(m6502 *cpu) {
    cpu->A |= get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void and(m6502 *cpu) {
    cpu->A &= get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

void eor(m6502 *cpu) {
    cpu->A ^= get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, cpu->A);
    update_sign_flag(cpu, cpu->A);
    PC += addr_mode_size(OPCODE);
}

/* Shifts & Rotate */

void asl(m6502 *cpu) {
    uint8_t value = get_mode_value(cpu, OPCODE);
    // Carry flag
    if (value & 0x80) set_flag_mask(cpu, CF);
    else reset_flag_mask(cpu, CF);
    // Shifting
    value = value << 1;
    // Zero and sign flags
    update_zero_flag(cpu, value);
    update_sign_flag(cpu, value);
    // Save the result
    set_mode_value(cpu, OPCODE, value);
    PC += addr_mode_size(OPCODE);
}

void lsr(m6502 *cpu) {
    uint8_t value = get_mode_value(cpu, OPCODE);
    // Carry flag
    if (value & 0x01) set_flag_mask(cpu, CF);
    else reset_flag_mask(cpu, CF);
    // Shifting
    value = value >> 1;
    // Zero and sign flags
    update_zero_flag(cpu, value);
    reset_flag_mask(cpu, NF);
    // Save the result
    set_mode_value(cpu, OPCODE, value);
    PC += addr_mode_size(OPCODE);
}

void rol(m6502 *cpu) {
    uint8_t value = get_mode_value(cpu, OPCODE);

    // Carry flag for later
    bool carry_after = value & 0x80;

    // Shifting
    value = (value << 1) | check_flag_mask(cpu, CF);

    // Zero and sign flags
    update_zero_flag(cpu, value);
    update_sign_flag(cpu, value);

    // Setting carry flag after the old one is used
    if (carry_after) set_flag_mask(cpu, CF);
    else reset_flag_mask(cpu, CF);

    // Save the result
    set_mode_value(cpu, OPCODE, value);
    PC += addr_mode_size(OPCODE);
}

void ror(m6502 *cpu) {
    uint8_t value = get_mode_value(cpu, OPCODE);

    // Carry flag for later
    bool carry_after = value & 0x01;

    // Shifting
    value = (value >> 1) | (check_flag_mask(cpu, CF) << 7);
    
    // Zero and sign flags
    update_zero_flag(cpu, value);
    update_sign_flag(cpu, value);

    // Setting carry flag after the old one is used
    if (carry_after) set_flag_mask(cpu, CF);
    else reset_flag_mask(cpu, CF);

    // Save the result
    set_mode_value(cpu, OPCODE, value);
    PC += addr_mode_size(OPCODE);
}

/* Flag */

void clc(m6502 *cpu) {
    reset_flag_mask(cpu, CF);
    PC += addr_mode_size(OPCODE);
}

void cli(m6502 *cpu) {
    reset_flag_mask(cpu, IF);
    PC += addr_mode_size(OPCODE);
}

void clv(m6502 *cpu) {
    reset_flag_mask(cpu, VF);
    PC += addr_mode_size(OPCODE);
}

void cld(m6502 *cpu) {
    reset_flag_mask(cpu, DF);
    PC += addr_mode_size(OPCODE);
}

void sec(m6502 *cpu) {
    set_flag_mask(cpu, CF);
    PC += addr_mode_size(OPCODE);
}

void sed(m6502 *cpu) {
    set_flag_mask(cpu, DF);
    PC += addr_mode_size(OPCODE);
}

void sei(m6502 *cpu) {
    set_flag_mask(cpu, IF);
    PC += addr_mode_size(OPCODE);
}

/* Comparisons */

static inline void compare(m6502 *cpu, uint8_t reg, uint8_t operand) {
    uint16_t diff = (uint16_t)reg - operand;

    // Carry flag
    if (reg >= operand) set_flag_mask(cpu, CF);
    else reset_flag_mask(cpu, CF);

    uint8_t result = (uint8_t)diff;

    update_zero_flag(cpu, result);
    update_sign_flag(cpu, result);

}

void cmp(m6502 *cpu) {
    uint8_t operand = get_mode_value(cpu, OPCODE);
    compare(cpu, cpu->A, operand);
    PC += addr_mode_size(OPCODE);
}

void cpx(m6502 *cpu) {
    uint8_t operand = get_mode_value(cpu, OPCODE);
    compare(cpu, cpu->X, operand);
    PC += addr_mode_size(OPCODE);
}

void cpy(m6502 *cpu) {
    uint8_t operand = get_mode_value(cpu, OPCODE);
    compare(cpu, cpu->Y, operand);
    PC += addr_mode_size(OPCODE);
}

/* Conditional Branch */

static inline void branch_if(m6502 *cpu, uint8_t condition) {
    if (condition) PC += addr_mode_size(OPCODE) + (int8_t)get_mode_value(cpu, OPCODE);
    else PC += addr_mode_size(OPCODE);
}

void bcc(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, CF) == 0);
}

void bcs(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, CF) == 1);
}

void beq(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, ZF) == 1);
}

void bmi(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, NF) == 1);
}

void bne(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, ZF) == 0);
}

void bpl(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, NF) == 0);
}

void bvc(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, VF) == 0);
}

void bvs(m6502 *cpu) {
    branch_if(cpu, check_flag_mask(cpu, VF) == 1);
}

/* Jumps & Subroutines */

void jmp(m6502 *cpu) {
    PC = FETCH16(cpu, PC + 1);;
}

void jsr(m6502 *cpu) {
    push16(cpu, (PC + 2) - 1);
    PC = FETCH16(cpu, PC + 1);;
}

void rts(m6502 *cpu) {
    PC = pull16(cpu) + 1;
}


/* Interrupts */

void brk(m6502 *cpu) {
    push16(cpu, PC + 2);
    push(cpu, cpu->sr | BF | 0b00100000);   // Push SR with break flag and bit 5 set to 1
    set_flag_mask(cpu, IF);
    PC = FETCH16(cpu, 0xFFFE);
}

void rti(m6502 *cpu) {
    cpu->sr = (pull(cpu) & 0b11011111) | 0b00100000;
    PC = pull16(cpu) + 1;
}

/* Other */

void illegal(m6502 *cpu) {
    perror("Illegal opcode");
    exit(1);
}

void bit(m6502 *cpu) {
    uint8_t operand = get_mode_value(cpu, OPCODE);
    update_zero_flag(cpu, operand & cpu->A);
    reset_flag_mask(cpu, NF | VF);
    cpu->sr |= operand & 0b11000000;    // Copy bit 6 and 7 from operand
    PC += addr_mode_size(OPCODE);
}

void nop(m6502 *cpu) {
    PC += addr_mode_size(OPCODE);
}

/* -----Initialize opcode dispatch table----- */

typedef void (*Insn)(m6502 *cpu);

static Insn const insn_table[256] = {
/*      0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F*/
/* 0 */ brk,    ora,    illegal,illegal,illegal,ora,    asl,    illegal,php,    ora,    asl,    illegal,illegal,ora,    asl,    illegal,
/* 1 */ bpl,    ora,    illegal,illegal,illegal,ora,    asl,    illegal,clc,    ora,    illegal,illegal,illegal,ora,    asl,    illegal,
/* 2 */ jsr,    and,    illegal,illegal,bit,    and,    rol,    illegal,plp,    and,    rol,    illegal,bit,    and,    rol,    illegal,
/* 3 */ bmi,    and,    illegal,illegal,illegal,and,    rol,    illegal,sec,    and,    illegal,illegal,illegal,and,    rol,    illegal,
/* 4 */ rti,    eor,    illegal,illegal,illegal,eor,    lsr,    illegal,pha,    eor,    lsr,    illegal,jmp,    eor,    lsr,    illegal,
/* 5 */ bvc,    eor,    illegal,illegal,illegal,eor,    lsr,    illegal,cli,    eor,    illegal,illegal,illegal,eor,    lsr,    illegal,
/* 6 */ rts,    adc,    illegal,illegal,illegal,adc,    ror,    illegal,pla,    adc,    ror,    illegal,jmp,    adc,    ror,    illegal,
/* 7 */ bvs,    adc,    illegal,illegal,illegal,adc,    ror,    illegal,sei,    adc,    illegal,illegal,illegal,adc,    ror,    illegal,
/* 8 */ illegal,sta,    illegal,illegal,sty,    sta,    stx,    illegal,dey,    illegal,txa,    illegal,sty,    sta,    stx,    illegal,
/* 9 */ bcc,    sta,    illegal,illegal,sty,    sta,    stx,    illegal,tya,    sta,    txs,    illegal,illegal,sta,    illegal,illegal,
/* A */ ldy,    lda,    ldx,    illegal,ldy,    lda,    ldx,    illegal,tay,    lda,    tax,    illegal,ldy,    lda,    ldx,    illegal,
/* B */ bcs,    lda,    illegal,illegal,ldy,    lda,    ldx,    illegal,clv,    lda,    tsx,    illegal,ldy,    lda,    ldx,    illegal,
/* C */ cpy,    cmp,    illegal,illegal,cpy,    cmp,    dec,    illegal,iny,    cmp,    dex,    illegal,cpy,    cmp,    dec,    illegal,
/* D */ bne,    cmp,    illegal,illegal,illegal,cmp,    dec,    illegal,cld,    cmp,    illegal,illegal,illegal,cmp,    dec,    illegal,
/* E */ cpx,    sbc,    illegal,illegal,cpx,    sbc,    inc,    illegal,inx,    sbc,    nop,    illegal,cpx,    sbc,    inc,    illegal,
/* F */ beq,    sbc,    illegal,illegal,illegal,sbc,    inc,    illegal,sed,    sbc,    illegal,illegal,illegal,sbc,    inc,    illegal
};

/* ----- Setup ----- */

void reset(m6502 *cpu) {
    cpu->sp = 0xFD;
    cpu->sr = 0x20 | IF;    // Bit 5 and irq disable set to one
    
    // Allocate RAM for cpu
    cpu->memory = malloc(sizeof(uint8_t) * 0x10000);
    
    // Reset vector
    PC = FETCH16(cpu, 0xFFFC);
}

void load_rom_to_ram(m6502 *cpu, const char *filename, uint16_t ram_location) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Cannot open rom file\n");
        exit(1);
    }

    // Get ROM size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);

    rewind(file);

    // Read the content and stores it at locations
    fread(&cpu->memory[ram_location], 1, file_size, file);

    fclose(file);
}

/* ----- Execution ----- */

void request_interrupt(m6502 *cpu, bool maskable) {
    if (maskable) {
        // IRQ
        cpu->irq = true;
    } else {
        // NMI
        cpu->nmi = true;
    }
}

static void handle_interrupt(m6502 *cpu) {
    if (cpu->nmi) {
        push16(cpu, PC);
        push(cpu, cpu->sr | 0b00100000);   // Push SR with bit 5 set to 1
        set_flag_mask(cpu, IF);
        cpu->nmi = false;
        PC = FETCH16(cpu, 0xFFFA);
    } else if (cpu->irq && !check_flag_mask(cpu, IF)) {
        push16(cpu, PC);
        push(cpu, cpu->sr | 0b00100000);   // Push SR with bit 5 set to 1
        set_flag_mask(cpu, IF);
        cpu->irq = false;
        PC = FETCH16(cpu, 0xFFFE);
    }
}

void execute_instr(m6502 *cpu) {
    handle_interrupt(cpu);
    uint8_t opcode = FETCH(cpu, PC);
    insn_table[opcode](cpu);
}
