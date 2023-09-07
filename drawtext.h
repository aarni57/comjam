#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include "minmax.h"
#include "font.h"

static void blit_char(int16_t x, int16_t y,
    uint16_t src_x, uint16_t src_y,
    uint8_t color_start) {
    uint16_t width = FONT6X6_WIDTH;
    uint16_t height = FONT6X6_HEIGHT;

    if (x + (int16_t)width <= 0 || x >= (int16_t)320 ||
        y + (int16_t)height <= 0 || y >= (int16_t)200) {
        return;
    }

    if (x < 0) {
        src_x -= x;
        width += x;
        x = 0;
    }

    if (y < 0) {
        src_y -= y;
        height += y;
        y = 0;
    }

    width = minu16(FONT6X6_WIDTH, 320 - (uint16_t)x);
    height = minu16(FONT6X6_HEIGHT, 200 - (uint16_t)y);

    {
        uint8_t __far* tgt = dblbuf + ((uint16_t)x + (uint16_t)y * 320);
        const uint8_t* src = FONT6X6_DATA + (src_x + src_y * FONT6X6_WIDTH);

        while (height--) {
            uint16_t row = width;
            while (row--) {
                const uint8_t c = *src++;
                if (c) {
                    *tgt = c + color_start;
                }
                
                tgt++;
            }

            tgt += 320 - width;
            src += FONT6X6_WIDTH - width;
        }
    }
}
static void draw_text(const char* text, int16_t left, int16_t top,
    uint8_t color_start) {
    int16_t x = left;
    int16_t y = top;

    const font_t* font = &FONT6X6;

    while (1) {
        const char c = *text++;
        if (!c) {
            break;
        }

        if (c == '\n') {
            x = left;
            y += font->line_height;
        } else {
            if (c >= FONT_FIRST_CHARACTER &&
                c <= FONT_LAST_CHARACTER) {
                const uint8_t character_index = (uint8_t)(c - FONT_FIRST_CHARACTER);
                const uint16_t font_y = (uint16_t)character_index * font->height;
                blit_char(x, y, 0, font_y, color_start);
                x += font->spacing + font->spacings[character_index];
            } else if (c == ' ') {
                x += font->space_width;
            }
        }
    }
}

#endif
