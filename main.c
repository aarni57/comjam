#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#include "vga.h"
#include "minmax.h"

int kb_clear_buffer();
#pragma aux kb_clear_buffer =   \
"mov ax, 0x0c00" \
"int 0x21";

void putz(const char* str);
#pragma aux putz = \
"l:" \
"mov dl, [bx]" \
"test dl, dl" \
"jz end" \
"push bx" \
"mov ah, 02h" \
"int 21h" \
"pop bx" \
"add bx, 1" \
"jmp l" \
"end:" \
parm [bx];

void set_text_cursor(uint8_t row, uint8_t col);
#pragma aux set_text_cursor = \
"mov ah, 2" \
"mov bh, 0" \
"xor al, al" \
"int 10h" \
parm [dh] [dl];

static uint8_t __far* dblbuf = NULL;

int main() {
    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        return 0;
    }

    {
        uint8_t __far* tgt = dblbuf;
        uint16_t x, y;
        for (y = 0; y < SCREEN_HEIGHT; ++y) {
            for (x = 0; x < SCREEN_WIDTH; ++x) {
                *tgt++ = y * 63 / SCREEN_HEIGHT;
            }
        }
    }

    vga_set_mode(0x13);

    {
        uint8_t i, r, g, b;
        for (i = 0; i < 64; ++i) {
            r = minu8(i * 3 / 2, 63);
            g = i > 32 ? minu8((i - 32) << 1, 63) : 0;
            b = i > 52 ? minu8((i - 52) << 2, 63) : 0;
            vga_set_palette(i, r, g, b);
        }
    }

    {
        int quit = 0;
        while (!quit) {
            if (kbhit()) {
                quit = 1;
            }

            vga_wait_for_retrace();
            _fmemcpy(VGA, dblbuf, SCREEN_NUM_PIXELS);
        }
    }

    vga_set_mode(0x3);
    kb_clear_buffer();

    putz("JEMM unloaded... Not really :-)\n");
    set_text_cursor(2, 0);

    return 0;
}
