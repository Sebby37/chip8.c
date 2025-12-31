#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

#define DISPLAY_W 64
#define DISPLAY_H 32
#define DISPLAY_GET(d,x,y) ((d[y] >> (63-x)) & 0x1)
#define DISPLAY_SET(d,x,y) ((d)[y] |=  (1ULL << (63-x)))
#define DISPLAY_CLR(d,x,y) ((d)[y] &= ~(1ULL << (63-x)))
#define DISPLAY_WRITE(d,x,y,p) (p ? DISPLAY_SET(d,x,y) : DISPLAY_CLR(d,x,y))

extern const u16 font_addr;
extern const u8 font[];

struct chip8 {
    u8 mem[0x1000];
    
    u16 pc;
    u16 i;
    
    u16 stack[48];
    u8 sp;

    u64 display[((DISPLAY_W / 8) / sizeof(u64)) * DISPLAY_H]; // Using a long for each row

    u8 delay;
    u8 sound;

    u8 v[16];
};

// Initialize a CHIP-8 struct
void chip_init(struct chip8 *chip);

// Load a program into the CHIP-8 given a path to the file
void chip_load(struct chip8 *chip, const char *program);

// Execute one instruction cycle. Returns a bool as to whether the screen should be redrawn
bool chip_cycle(struct chip8 *chip, u16 key_mask);
