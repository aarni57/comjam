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
            aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
            c = *tgt;
            *tgt++ = c + 128;
        }

        tgt += row_advance;
    }
}

static inline void draw_hline(int16_t x0, int16_t x1, int16_t y, uint8_t c) {
    uint8_t __far* tgt, __far* tgt_end;
    aw_assert(x0 <= x1);

    if (y < 0 || y > SCREEN_Y_MAX || x0 > SCREEN_X_MAX || x1 < 0)
        return;

    x0 = clamp16(x0, 0, SCREEN_X_MAX);
    x1 = clamp16(x1, 0, SCREEN_X_MAX);

    tgt = dblbuf + mul_by_screen_stride(y) + x0;
    aw_assert(tgt >= dblbuf);
    tgt_end = tgt + (x1 - x0 + 1);
    while (tgt < tgt_end) {
        aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
        *tgt++ = c;
    }
}

static inline void draw_hline_no_check(int16_t x0, int16_t x1, int16_t y, uint8_t c) {
    uint8_t __far* tgt, __far* tgt_end;
    aw_assert(x0 <= x1);
    tgt = dblbuf + mul_by_screen_stride(y) + x0;
    aw_assert(tgt >= dblbuf);
    tgt_end = tgt + (x1 - x0 + 1);
    while (tgt < tgt_end) {
        aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
        *tgt++ = c;
    }
}

static inline void draw_darkened_hline_no_check(int16_t x0, int16_t x1, int16_t y) {
    uint8_t __far* tgt, __far* tgt_end;
    aw_assert(x0 <= x1);
    tgt = dblbuf + mul_by_screen_stride(y) + x0;
    aw_assert(tgt >= dblbuf);
    tgt_end = tgt + (x1 - x0 + 1);
    while (tgt < tgt_end) {
        aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
        *tgt += 128;
        tgt++;
    }
}

static inline void draw_vline(int16_t x, int16_t y0, int16_t y1, uint8_t c) {
    uint8_t __far* tgt, __far* tgt_end;
    aw_assert(y0 <= y1);

    if (x < 0 || x > SCREEN_X_MAX || y0 > SCREEN_Y_MAX || y1 < 0)
        return;

    y0 = clamp16(y0, 0, SCREEN_Y_MAX);
    y1 = clamp16(y1, 0, SCREEN_Y_MAX);

    tgt = dblbuf + mul_by_screen_stride(y0) + x;
    aw_assert(tgt >= dblbuf);
    tgt_end = tgt + mul_by_screen_stride(y1 - y0 + 1);
    while (tgt < tgt_end) {
        aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
        *tgt = c;
        tgt += SCREEN_STRIDE;
    }
}

static inline void draw_vline_no_check(int16_t x, int16_t y0, int16_t y1, uint8_t c) {
    uint8_t __far* tgt, __far* tgt_end;
    aw_assert(y0 <= y1);
    tgt = dblbuf + mul_by_screen_stride(y0) + x;
    aw_assert(tgt >= dblbuf);
    tgt_end = dblbuf + mul_by_screen_stride(y1 + 1) + x;
    while (tgt < tgt_end) {
        aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
        *tgt = c;
        tgt += SCREEN_STRIDE;
    }
}

static inline void draw_darkened_vline_no_check(int16_t x, int16_t y0, int16_t y1) {
    uint8_t __far* tgt, __far* tgt_end;
    aw_assert(y0 <= y1);
    tgt = dblbuf + mul_by_screen_stride(y0) + x;
    aw_assert(tgt >= dblbuf);
    tgt_end = dblbuf + mul_by_screen_stride(y1 + 1) + x;
    while (tgt < tgt_end) {
        aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
        *tgt += 128;
        tgt += SCREEN_STRIDE;
    }
}

static inline void draw_box_outline_no_check(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c) {
    draw_hline_no_check(x0, x1, y0, c);
    draw_vline_no_check(x0, y0 + 1, y1 - 1, c);
    draw_vline_no_check(x1, y0 + 1, y1 - 1, c);
    draw_hline_no_check(x0, x1, y1, c);
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
            while (tgt < tgt_end) {
                aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
                *tgt++ = c;
            }
        }

        if (y1 >= 0 && y1 < SCREEN_HEIGHT) {
            tgt = dblbuf + mul_by_screen_stride(y1) + left;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + (right - left + 1);
            while (tgt < tgt_end) {
                aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
                *tgt++ = c;
            }
        }
    }

    if (bottom - top > 1) {
        if (x0 >= 0 && x0 < SCREEN_WIDTH) {
            tgt = dblbuf + mul_by_screen_stride(top + 1) + x0;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + mul_by_screen_stride(bottom - top - 1);
            while (tgt < tgt_end) {
                aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
                *tgt = c;
                tgt += SCREEN_WIDTH;
            }
        }

        if (x1 >= 0 && x1 < SCREEN_WIDTH) {
            tgt = dblbuf + mul_by_screen_stride(top + 1) + x1;
            aw_assert(tgt >= dblbuf);
            tgt_end = tgt + mul_by_screen_stride(bottom - top - 1);
            while (tgt < tgt_end) {
                aw_assert(tgt < dblbuf + SCREEN_NUM_PIXELS);
                *tgt = c;
                tgt += SCREEN_WIDTH;
            }
        }
    }
}

static inline void draw_box_outline_corners(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c) {
    draw_hline(x0, x0 + ((x1 - x0) >> 2), y0, c);
    draw_hline(x1 - ((x1 - x0) >> 2), x1, y0, c);
    draw_hline(x0, x0 + ((x1 - x0) >> 2), y1, c);
    draw_hline(x1 - ((x1 - x0) >> 2), x1, y1, c);
    draw_vline(x0, y0 + 1, y0 + ((y1 - y0) >> 2), c);
    draw_vline(x0, y1 - ((y1 - y0) >> 2), y1 - 1, c);
    draw_vline(x1, y0 + 1, y0 + ((y1 - y0) >> 2), c);
    draw_vline(x1, y1 - ((y1 - y0) >> 2), y1 - 1, c);
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

static inline void put_pixel(int16_t x, int16_t y, uint8_t c) {
    if (x < 0 || y < 0 || x > SCREEN_X_MAX || y > SCREEN_Y_MAX)
        return;

    dblbuf[mul_by_screen_stride(y) + x] = c;
}

static inline void draw_noise(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t r) {
    int16_t i, j;
    uint8_t __far* tgt = dblbuf + (mul_by_screen_stride(y) + x);
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            uint8_t c = r & 7;
            if (c >= 5)
                *tgt = c - 5;
            tgt++;
#if defined(INLINE_ASM)
            __asm {
                mov ax, r
                mov bx, ax
                shl bx, 7
                xor ax, bx
                mov bx, ax
                shr bx, 9
                xor ax, bx
                mov bx, ax
                shl bx, 8
                xor ax, bx
                mov r, ax
            }
#else
            r ^= r << 7;
            r ^= r >> 9;
            r ^= r << 8;
#endif
        }

        tgt += SCREEN_STRIDE - width;
    }
}

static inline void blit_image(int16_t x, int16_t y, int16_t src_x, int16_t src_y,
    int16_t width, int16_t height, uint8_t __far* src) {
    int16_t i, j;
    uint8_t __far* tgt = dblbuf + (mul_by_screen_stride(y) + x);
    src += mul_by_screen_stride(src_y) + src_x;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            uint8_t c = *src++;
            if (c)
                *tgt = c;
            tgt++;
        }

        tgt += SCREEN_STRIDE - width;
        src += SCREEN_STRIDE - width;
    }
}

#endif
