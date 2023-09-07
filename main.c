#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#include "vga.h"
#include "minmax.h"
#include "tri.h"

//

static uint8_t __far* dblbuf = NULL;

#include "drawtext.h"

//

static uint16_t opl_base = 0x388;

#include "opl.h"

//

int kb_clear_buffer();
#pragma aux kb_clear_buffer =   \
"mov ax, 0x0c00" \
"int 0x21" \
modify [ax];

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
modify [ax dx] \
parm [bx];

void set_text_cursor(uint8_t row, uint8_t col);
#pragma aux set_text_cursor = \
"mov ah, 2" \
"mov bh, 0" \
"xor al, al" \
"int 10h" \
modify [ax bh] \
parm [dh] [dl];

const char* exit_message = "JEMM unloaded... Not really :-)";

void main() {
    opl_init();

    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        goto exit;
    }

    _fmemset(dblbuf, 0, 320 * 200);

#if 0
    {
        uint8_t __far* tgt = dblbuf;
        uint16_t x, y;
        for (y = 0; y < SCREEN_HEIGHT; ++y) {
            uint8_t c = y * 63 / SCREEN_HEIGHT;
            for (x = 0; x < SCREEN_WIDTH; ++x) {
                *tgt++ = c;
            }
        }
    }
#endif

    vga_set_mode(0x13);
    vga_set_palette(253, 0, 20, 0);
    vga_set_palette(254, 0, 48, 0);
    vga_set_palette(255, 0, 58, 0);

#if 0
    {
        uint8_t i, r, g, b;
        for (i = 0; i < 64; ++i) {
            r = minu8(i * 3 / 2, 63);
            g = i > 32 ? minu8((i - 32) << 1, 63) : 0;
            b = i > 52 ? minu8((i - 52) << 2, 63) : 0;
            vga_set_palette(i, r, g, b);
        }
    }
#endif

    {
        int quit = 0;
        while (!quit) {
            if (kbhit()) {
                char ch = getch();
                switch (ch) {
                    case 27:
                        quit = 1;
                        break;

                    case 'a':
                        opl_play();
                        break;

                    default:
                        break;
                }
            }

#if 0
            {
                int32_t x0, y0, x1, y1, x2, y2;
                static uint8_t c = 0;
                static int32_t x = 0;

                c++;
                x++;
                x &= 255;

                x0 = x;
                y0 = 10;
                x1 = x + 40;
                y1 = 160;
                x2 = x + 240;
                y2 = 180;

                x0 <<= PROJECTION_SUBPIXEL_BITS;
                y0 <<= PROJECTION_SUBPIXEL_BITS;
                x1 <<= PROJECTION_SUBPIXEL_BITS;
                y1 <<= PROJECTION_SUBPIXEL_BITS;
                x2 <<= PROJECTION_SUBPIXEL_BITS;
                y2 <<= PROJECTION_SUBPIXEL_BITS;

                tri(x0, y0, x1, y1, x2, y2, c, dblbuf);
            }
#endif

            draw_text("Prerendering graphics...", 6, 6, 252);
            draw_text_cursor(6, 12, 255);

            vga_wait_for_retrace();
            _fmemcpy(VGA, dblbuf, SCREEN_NUM_PIXELS);
        }
    }

    _ffree(dblbuf);

exit:
    opl_done();
    vga_set_mode(0x3);
    putz(exit_message);
    set_text_cursor(2, 0);
    kb_clear_buffer();
}
