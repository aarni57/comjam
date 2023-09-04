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

static uint16_t opl_base = 0x388;

static inline void opl_write(uint8_t reg, uint8_t v) {
    outp(opl_base, reg);
    inp(opl_base);
    outp(opl_base + 1, v);
    inp(opl_base);

#if 0
    if (opl == 3) {
        outp(opl_base + 2, reg);
        inp(opl_base);
        outp(opl_base + 2 + 1, v);
        inp(opl_base);
    }
#endif
}

static void opl_reset() {
    uint8_t i;
    for (i = 0x01; i <= 0xf5; i++) {
        opl_write(i, 0);
    }
}

static void opl_init() {
    uint8_t val1, val2;

    // Reset timer 1 and 2
    opl_write(0x4, 0x60);

    // Reset IRQ
    opl_write(0x4, 0x80);

    // Read status
    val1 = inp(opl_base);

    // Set timer 1 to 0xff
    opl_write(0x2, 0xff);

    // Start timer 1
    opl_write(0x4, 0x21);

    // Delay for more than 80us, pick 10ms
    delay(10);

    // Read status
    val2 = inp(opl_base);

    // Reset timer 1 and 2
    opl_write(0x4, 0x60);

    // Reset IRQ
    opl_write(0x4, 0x80);

    if ((val1 & 0xe0) != 0x00 || (val2 & 0xe0) != 0xc0) {
        return;
    }

#if 0
    // OPL3 detection (not used)
    val1 = inp(opl_base);

    if ((val1 & 0x06) == 0x00) {
        opl = 3;
    } else {
        opl = 2;
    }
#endif

    opl_reset();
    opl_write(0x01, 0x20);
    opl_write(0x08, 0x40);
}

static void opl_done() {
    opl_reset();
}

static void opl_play() {
    uint8_t voice = 0;

    opl_write(0xb0, 0);

    opl_write(0x20, 0x01); // Set the modulator's multiple
    opl_write(0x40, 0x06); // Set the modulator's level
    opl_write(0x60, 0x11); // Modulator attack & decay
    opl_write(0x80, 0x11); // Modulator sustain & release
    opl_write(0xa0, 0x28); // Set voice frequency's LSB
    opl_write(0x23, 0x00); // Set the carrier's multiple
    opl_write(0x43, 0x00); // Set the carrier to maximum volume
    opl_write(0x63, 0x11); // Carrier attack & decay
    opl_write(0x83, 0x11); // Carrier sustain & release
    opl_write(0xb0, 0x20 | 0x10); // Turn the voice on; set the octave and freq MSB
}

const char* exit_message = "JEMM unloaded... Not really :-)";

static uint8_t __far* dblbuf = NULL;

void main() {
    opl_init();

    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        goto exit;
    }

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

            {
                int32_t x0, y0, x1, y1, x2, y2;
                static uint8_t c = 0;
                static int32_t x = 0;

                c++;
                x++;
                x &= 255;

                x0 = x;
                y0 = 10;
                x1 = x + 20;
                y1 = 90;
                x2 = x + 100;
                y2 = 100;

                tri(x0, y0, x1, y1, x2, y2, c, dblbuf);
            }

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
