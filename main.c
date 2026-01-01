#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "chip8.h"

u16 get_width() {
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != 0) {
		perror("ioctl");
		exit(1);
	}
	return w.ws_col;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: ./chip8 PROGRAM\n\tPROGRAM\tPath to a .ch8 program to be run");
        return 1;
    }
    
    struct chip8 chip;
    chip_init(&chip);
    chip_load(&chip, argv[1]);

    int width = get_width();
    if (width > DISPLAY_W)
    	width = DISPLAY_W;

    for (int i = 0; i < 100; i++) {
        if (!chip_cycle(&chip, 0))
            continue;
        //printf("\e[1;1H\e[2J");
        for (int y = 0; y < DISPLAY_H; y++) {
            for (int x = 0; x < width; x++)
                if (DISPLAY_GET(chip.display, x, y))
                    putc('#', stdout);
                else
                    putc(' ', stdout);
            putc('\n', stdout);
        }
    }

    return 0;
}
