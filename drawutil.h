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

static inline void draw_box_outline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c) {
    int16_t left = clamp16(x0, 0, SCREEN_WIDTH - 1);
    int16_t right = clamp16(x1, 0, SCREEN_WIDTH - 1);
    int16_t top = clamp16(y0, 0, SCREEN_HEIGHT - 1);
    int16_t bottom = clamp16(y1, 0, SCREEN_HEIGHT - 1);
    int16_t x, y;
    uint8_t __far* tgt, __far* tgt_end;

    if (x0 < SCREEN_WIDTH && x1 >= 0) {
        if (y0 >= 0 && y0 < SCREEN_HEIGHT) {
            tgt = dblbuf + mul_by_screen_stride(y0) + left;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + (right - left + 1);
            aw_assert(tgt_end <= dblbuf + SCREEN_NUM_PIXELS);
            while (tgt < tgt_end) {
                *tgt++ = c;
            }
        }

        if (y1 >= 0 && y1 < SCREEN_HEIGHT) {
            tgt = dblbuf + mul_by_screen_stride(y1) + left;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + (right - left + 1);
            aw_assert(tgt_end <= dblbuf + SCREEN_NUM_PIXELS);
            while (tgt < tgt_end) {
                *tgt++ = c;
            }
        }
    }

    if (bottom - top > 1) {
        if (x0 >= 0 && x0 < SCREEN_WIDTH) {
            tgt = dblbuf + mul_by_screen_stride(top + 1) + x0;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + mul_by_screen_stride(bottom - top - 1);
            aw_assert(tgt_end <= dblbuf + SCREEN_NUM_PIXELS);
            while (tgt < tgt_end) {
                *tgt = c;
                tgt += SCREEN_WIDTH;
            }
        }

        if (x1 >= 0 && x1 < SCREEN_WIDTH) {
            tgt = dblbuf + mul_by_screen_stride(top + 1) + x1;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + mul_by_screen_stride(bottom - top - 1);
            aw_assert(tgt_end <= dblbuf + SCREEN_NUM_PIXELS);
            while (tgt < tgt_end) {
                *tgt = c;
                tgt += SCREEN_WIDTH;
            }
        }
    }
}

static inline void draw_cross(uint16_t x, uint16_t y, uint8_t c) {
    uint8_t __far* tgt = dblbuf + mul_by_screen_stride(y) + x;
    tgt[-SCREEN_STRIDE * 3] = c;
    tgt[-SCREEN_STRIDE * 2] = c;
    tgt[-3] = c;
    tgt[-2] = c;
    tgt[2] = c;
    tgt[3] = c;
    tgt[SCREEN_STRIDE * 2] = c;
    tgt[SCREEN_STRIDE * 3] = c;
}

#endif
