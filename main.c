#include <stdio.h>
#include <time.h>
#include "src/lib6502.h"

int main(int argc, char *argv[]) {
    // Initialize a new cpu
    m6502 cpu;
    init(&cpu);
    load_rom_to_ram(&cpu, "roms/6502_functional_test.bin", 0);
    reset(&cpu);
    cpu.pc = 0x0400;

    add_break_point(0x3469);

    // Execution
    while (1) {
        execute_instr(&cpu, false);
    }

    return 0;
}
