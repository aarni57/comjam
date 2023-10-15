#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include "minmax.h"
#include "font.h"

static void blit_char(int16_t x, int16_t y,
    uint16_t src_x, uint16_t src_y,
    uint8_t color_start, uint8_t color_max) {
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

    width = minu16(FONT6X6_WIDTH, 320 - x);
    height = minu16(FONT6X6_HEIGHT, 200 - y);

    {
        uint8_t __far* tgt = dblbuf + (x + y * 320);
        const uint8_t* src = FONT6X6_DATA;
        uint16_t index = src_x + src_y * FONT6X6_WIDTH;
        uint8_t step = index & 3;
        uint8_t c;

        src += index >> 2;
        c = *src;

        while (height--) {
            uint16_t row = width;
            while (row--) {
                uint8_t c2 = (c >> (step << 1)) & 3;
                if (c2) {
                    if (c2 > color_max) c2 = color_max;
                    *tgt = c2 + color_start;
                }

                tgt++;
                step++;
                if (step == 4) {
                    step = 0;
                    src++;
                    c = *src;
                }
            }

            tgt += 320 - width;
            step += FONT6X6_WIDTH - width;
            if (step >= 4) {
                while (step >= 4) {
                    src++;
                    step -= 4;
                }

                c = *src;
            }
        }
    }
}

static int16_t draw_text_impl(const char* text, int16_t left, int16_t top,
    uint8_t color_start, uint8_t color_max) {
    int16_t x = left;
    int16_t y = top;

    const font_t* font = &FONT6X6;

    for (;;) {
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
                uint8_t character_index = (uint8_t)(c - FONT_FIRST_CHARACTER);
                uint16_t font_y = (uint16_t)character_index * font->height;
                blit_char(x, y, 0, font_y, color_start, color_max);
                x += font->spacing + font->spacings[character_index];
            } else if (c == ' ') {
                x += font->space_width;
            }
        }
    }

    return x;
}

static int16_t calc_text_width(const char* text) {
    int16_t line_width = 0;
    int16_t max_width = 0;

    const font_t* font = &FONT6X6;

    for (;;) {
        const char c = *text++;
        if (!c) {
            break;
        }

        if (c == '\n') {
            if (max_width < line_width)
                max_width = line_width;

            line_width = 0;
        } else {
            if (c >= FONT_FIRST_CHARACTER &&
                c <= FONT_LAST_CHARACTER) {
                uint8_t character_index = (uint8_t)(c - FONT_FIRST_CHARACTER);
                line_width += font->spacing + font->spacings[character_index];
            } else if (c == ' ') {
                line_width += font->space_width;
            }
        }
    }

    return line_width > max_width ? line_width : max_width;
}

static inline int16_t draw_text(const char* text, int16_t left, int16_t top,
    uint8_t color_start) {
    return draw_text_impl(text, left, top, color_start, 255);
}

static inline int16_t draw_dimmed_text(const char* text, int16_t left,
    int16_t top, uint8_t color_start) {
    return draw_text_impl(text, left, top, color_start, 2);
}

static void draw_centered_text(const char* text, int16_t x, int16_t y, uint8_t c) {
    int16_t text_width = calc_text_width(text);
    draw_dimmed_text(text, x - (text_width >> 1), y, c);
}

static void draw_text_cursor(int16_t left, int16_t top, uint8_t color) {
    uint8_t x, y;
    uint8_t __far* tgt = dblbuf + (left + top * 320);

    for (y = 0; y < 6; ++y) {
        for (x = 0; x < 5; ++x) {
            *tgt++ = color;
        }

        tgt += 320 - 5;
    }
}

#endif
