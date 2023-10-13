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
#define SCREEN_NUM_PIXELS 64000

#define SCREEN_X_MAX 319
#define SCREEN_Y_MAX 199

#define SCREEN_LOGICAL_HEIGHT 240

#define SCREEN_CENTER_X 160
#define SCREEN_CENTER_Y 100

#define SCREEN_STRIDE SCREEN_WIDTH

static inline uint16_t mul_by_screen_stride(uint16_t x) {
    return (x << 8) + (x << 6);
}

static inline int32_t mul_by_screen_stride32(int32_t x) {
    return (x << 8) + (x << 6);
}

//

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

static inline void vga_copy(uint8_t __far* src) {
#if !defined(INLINE_ASM)
    _fmemcpy(VGA, src, SCREEN_NUM_PIXELS);
#else
    __asm {
        push es
        push ds
        push edi
        push esi

        mov edx, src
        movzx esi, dx
        shr edx, 16
        mov ds, dx

        mov dx, 0xa000
        mov es, dx
        xor edi, edi

        mov cx, SCREEN_NUM_PIXELS
        shr cx, 2

        rep movsd

        pop esi
        pop edi
        pop ds
        pop es
    }
#endif
}

#endif
