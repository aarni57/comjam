#ifndef VGA_H
#define VGA_H

#include <conio.h>
#include <stdint.h>

#define VGA (uint8_t far*)0xa0000000L

#define VGA_INT 0x10

#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA 0x3C9

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_NUM_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)

#define VGA_INPUT_STATUS 0x3da
#define VGA_VRETRACE 0x08

void vga_set_mode(uint8_t mode);
#pragma aux vga_set_mode = \
"xor ah, ah" \
"int 0x10" \
modify [ax] \
parm [al];

static inline void vga_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outp(PALETTE_INDEX, index);
    outp(PALETTE_DATA, r);
    outp(PALETTE_DATA, g);
    outp(PALETTE_DATA, b);
}

static inline void vga_wait_for_retrace() {
    while (inp(VGA_INPUT_STATUS) & VGA_VRETRACE);
    while (!(inp(VGA_INPUT_STATUS) & VGA_VRETRACE));
}

#endif
