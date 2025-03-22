#include "_gb.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    gb_init();

    FILE* f = fopen("blargg_gb_tests/cpu_instrs/individual/09-op r,r.gb", "rb");
    if (!f) {
        printf("ROM not found\n");
        return 1;
    }

    uint8_t rom[0x8000];
    size_t read = fread(rom, 1, sizeof(rom), f);
    printf("Read %zu bytes from ROM\n", read);
    fclose(f);

    gb_load_rom(rom, sizeof(rom));

    for (int i = 0; i < 2000; i++) {
        gb_step_frame();
    }

    printf("\nDone\n");
    return 0;
}