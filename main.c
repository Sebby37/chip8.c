#include <stdio.h>
#include "chip8.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: ./chip8 PROGRAM\n\tPROGRAM\tPath to a .ch8 program to be run");
        return 1;
    }
    
    struct chip8 chip;
    chip_init(&chip);
    chip_load(&chip, argv[1]);

    while (1) {
        if (!chip_cycle(&chip, 0))
            continue;
        printf("\e[1;1H\e[2J");
        for (int y = 0; y < DISPLAY_H; y++) {
            for (int x = 0; x < DISPLAY_W; x++)
                if (DISPLAY_GET(chip.display, x, y))
                    putc('#', stdout);
                else
                    putc(' ', stdout);
            putc('\n', stdout);
        }
    }

    return 0;
}