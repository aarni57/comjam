#ifndef DRAWUTIL_H
#define DRAWUTIL_H

#include <stdint.h>

static inline void draw_darkened_box(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint16_t x, y, row_advance;
    uint8_t __far* tgt = dblbuf + mul_by_screen_stride(y0) + x0;
    uint8_t c;

    row_advance = SCREEN_STRIDE - (x1 - x0 + 1);

    for (y = y0; y <= y1; ++y) {
        for (x = x0; x <= x1; ++x) {
            c = *tgt;
            *tgt++ = c + 128;
        }

        tgt += row_advance;
    }
}

static inline void draw_box_outline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t c) {
    uint16_t x, y;
    uint8_t __far* tgt;

    tgt = dblbuf + mul_by_screen_stride(y0) + x0;

    for (x = x0; x <= x1; ++x) {
        *tgt++ = c;
    }

    tgt = dblbuf + mul_by_screen_stride(y1) + x0;

    for (x = x0; x <= x1; ++x) {
        *tgt++ = c;
    }

    if (y1 - y0 > 1) {
        uint16_t width = x1 - x0;
        tgt = dblbuf + mul_by_screen_stride(y0 + 1) + x0;
        for (y = y0; y < y1 - 1; ++y) {
            tgt[0] = c;
            tgt[width] = c;
            tgt += SCREEN_WIDTH;
        }
    }
}

#endif
