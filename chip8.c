#include "chip8.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const u16 font_addr = 0x050;
const u8 font[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip_init(struct chip8 *chip)
{
    chip->pc = 0x200;
    chip->sp = 0;
    chip->delay = 0;
    chip->sound = 0;

    srand(time(NULL));
    memcpy(&chip->mem[font_addr], font, sizeof(font));
    memset(chip->display, 0x00, sizeof(chip->display));
}

void chip_load(struct chip8 *chip, const char *program)
{
    FILE *f = fopen(program, "rb");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    fread(&chip->mem[0x200], 1, size, f);

    fclose(f);
}

bool chip_cycle(struct chip8 *chip, u16 key_mask)
{
    // Fetch
    u16 instruction = (chip->mem[chip->pc & 0xFFF] << 8) | chip->mem[(chip->pc+1) & 0xFFF];
    chip->pc += 2;
    chip->pc &= 0xFFF; // Bound to 12 bits

    // Decode
    u16 opcode = (instruction >> 12);
    u16  x     = (instruction >> 8) & 0x0F;
    u16   y    = (instruction >> 4) & 0x0F;
    u16    n   = instruction & 0x000F;
    u16   nn   = instruction & 0x00FF;
    u16  nnn   = instruction & 0x0FFF;

    printf("(%03d) Executing instruction %#06X\n", (int)chip->pc, instruction);
    
    // Execute
    u8 *v = chip->v; // Convenience
    bool redraw = false;
    switch (opcode) {
        case 0x0:
            switch (instruction) {
                case 0x00E0:
                    // Clear screen
                    memset(chip->display, 0x00, sizeof(chip->display));
                    redraw = true;
                    break;
                case 0x00EE:
                    // Return from subroutine
                    chip->pc = chip->stack[chip->sp--];
                    break;
            }
            break;
        case 0x1:
            // 1NNN: Jump to 0xNNN
            chip->pc = nnn;
            break;
        case 0x2:
            // 2NNN: Call subroutine at 0xNNN
            chip->stack[chip->sp++] = chip->pc;
            chip->pc = nnn;
            break;
        case 0x3:
            // 3XNN: Skip next if vX == NN
            if (v[x] == nn)
                chip->pc += 2;
            break;
        case 0x4:
            // 4XNN: Skip next if vX != NN
            if (v[x] != nn)
                chip->pc += 2;
            break;
        case 0x5:
            // 5XY0: Skip next if vX == vY
            if (v[x] == v[y])
                chip->pc += 2;
            break;
        case 0x6:
            // 6XNN: Set vX = NN
            v[x] = nn;
            break;
        case 0x7:
            // 7XNN: Add NN to vX, without carry
            v[x] += nn;
            break;
        case 0x8:
            // Arithmetic instructions, n conveniently also denotes which one!
            switch (n) {
                case 0x0:
                    // 8XY0: Set vX = vY
                    v[x] = v[y];
                    break;
                case 0x1:
                    // 8XY1: OR, vX |= vY
                    v[x] |= v[y];
                    break;
                case 0x2:
                    // 8XY2: AND, vX &= vY
                    v[x] &= v[y];
                    break;
                case 0x3:
                    // 8XY3: XOR, vX ^= vY
                    v[x] ^= v[y];
                    break;
                case 0x4:
                    // 8XY4: Add, vX += vY
                    v[x] += v[y];
                    v[0xF] = (v[x] < v[y]); // Overflow detection!
                    break;
                case 0x5:
                    // 8XY5: Subtract, vX -= vY
                    if (v[x] != v[y])
                        v[0xF] = (v[x] > v[y]);
                    v[x] -= v[y];
                    break;
                case 0x6:
                    // 8XY6: Shift, vX = vY >> 1, setting vF to the shifted out bit
                    // AMBIGUOUS!!! This is correct on CHIP8 but not on SUPER-CHIP
                    v[x] = v[y];
                    v[0xF] = (v[x] & 0x1);
                    v[x] >>= 1;
                    break;
                case 0x7:
                    // 8XY7: Subtract, vX = vY - vX
                    if (v[x] != v[y])
                        v[0xF] = (v[x] > v[y]);
                    v[x] = v[y] - v[x];
                    break;
                case 0xE:
                    // 8XYE: Shift, vX = vY << 1, setting vF to the shifted out bit
                    // AMBIGUOUS!!! This is correct on CHIP8 but not on SUPER-CHIP
                    v[x] = v[y];
                    v[0xF] = (v[x] & 0x80) >> 7;
                    v[x] <<= 1;
                    break;
            }
            break;
        case 0x9:
            // 9XY0: Skip next if vX != vY
            if (v[x] != v[y])
                chip->pc += 2;
            break;
        case 0xA:
            // ANNN: Set index
            chip->i = nnn;
            break;
        case 0xB:
            // BNNN: Jump with offset
            // AMBIGUOUS!!! Correct on CHIP8 but not on SUPERCHIP
            chip->pc = (nnn + v[0]) & 0xFFF;
            break;
        case 0xC:
            // CXNN: Random
            v[x] = rand() & nn;
            break;
        case 0xD:
            // DXYN: Display
            u8 sx = v[x] % DISPLAY_W,
               sy = v[y] % DISPLAY_H;
            v[0xF] = 0;
            printf("Drawing %d lines, starting at (%d,%d), where I=%d\n", (int)n, (int)sx, (int)sy, (int)chip->i);

            for (int r = 0; r < n; r++) {
                if (sy+r >= DISPLAY_H)
                    break;
                
                u8 row = chip->mem[chip->i + r];
                
                for (int c = 0; c < 8; c++) {
                    if (sx+c >= DISPLAY_W)
                        break;

                    u8 cur_pixel = DISPLAY_GET(chip->display, sx+c, sy+r);
                    u8 new_pixel = (row & (1 << c)) ? 1 : 0;
                    if (cur_pixel && new_pixel)
                        v[0xF] = 1;
                    
                    if (sx+c < DISPLAY_W)
                        DISPLAY_WRITE(chip->display, sx+c, sy+r, new_pixel ^ cur_pixel);
                }
            }

            redraw = true;
            break;
        case 0xE:
            // EXnn: Skip if key instructions, nn is already conveniently decoded!
            switch (nn) {
                case 0x9E:
                    // EX9E: Skip if key in vX is pressed
                    if ((key_mask & (1 << v[x])) > 0)
                        chip->pc += 2;
                    break;
                case 0xA1:
                    // EXA1: Skip if key in vX is NOT pressed
                    if ((key_mask & (1 << v[x])) == 0)
                        chip->pc += 2;
                    break;
            }
            break;
        case 0xF:
            // FXnn: Timers, index add, get key, font char, BCD convert and mem store/loads
            switch (nn) {
                // Timers
                case 0x07:
                    // FX07: Set vX = current value of delay timer
                    v[x] = chip->delay;
                    break;
                case 0x15:
                    // FX15: Set delay timer = vX
                    chip->delay = v[x];
                    break;
                case 0x18:
                    // FX18: Set sound timer = vX
                    chip->sound = v[x];
                    break;
                
                case 0x1E:
                    // FX1E: Add to index
                    // AMBIGUOUS: Just doing it the way the Amiga one did it
                    chip->i += v[x];
                    v[0xF] = (chip->i > 0x0FFF);
                    chip->i &= 0x0FFF;
                    break;
                
                case 0x0A:
                    // FX0A: Get key
                    if (key_mask == 0) {
                        chip->pc -= 2;
                    }
                    else {
                        // Find the pressed key
                        for (int i = 0; i < 16; i++) {
                            if (key_mask & (1 << i)) {
                                v[x] = i;
                                break;
                            }
                        }
                    }
                    break;
                
                case 0x29:
                    // FX29: Font character, i = &hex_char[X]
                    chip->i = font_addr + v[x]*5;
                    break;
                
                case 0x33:
                    // FX33: BCD convert
                    chip->mem[chip->i  ] =  v[x] / 100;
                    chip->mem[chip->i+1] = (v[x] / 10) % 10;
                    chip->mem[chip->i+2] =  v[x] % 10;
                    break;
                
                case 0x55:
                    // FX55: Store registers from v0-vX into mem starting from i
                    // MODERN: Does not increment i, so not compatible with original CHIP8
                    for (int i = 0; i <= x; i++)
                        chip->mem[chip->i + i] = v[i];
                    break;
                case 0x65:
                    // FX65: Load registers to v0-vX from mem starting from i
                    // MODERN: Does not increment i, so not compatible with original CHIP8
                    for (int i = 0; i <= x; i++)
                        v[i] = chip->mem[chip->i + i];
                    break;
            }
    }

    return redraw;
}
