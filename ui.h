#pragma once
#include "chip8.h"

// The GUI mainloop
void gui_main(struct chip8 *chip);

// Input handling wrapper, because ncurses is a pain for good input like I need
u16 gui_get_key_mask(u16 dt);