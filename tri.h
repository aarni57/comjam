#ifndef TRI_H
#define TRI_H

#include "minmax.h"
#include "screen.h"

//

#define RASTER_SCREEN_X_MAX (SCREEN_WIDTH - 1)
#define RASTER_SCREEN_Y_MAX (SCREEN_HEIGHT - 1)

#define RASTER_BLOCK_SIZE_SHIFT 3
#define RASTER_BLOCK_SIZE (1 << RASTER_BLOCK_SIZE_SHIFT)
#define RASTER_BLOCK_MASK (RASTER_BLOCK_SIZE - 1)

static inline int16_t mul_by_raster_block_mask(int16_t x) {
#if 1
    return (x << 3) - x;
#else
    return x * RASTER_BLOCK_MASK;
#endif
}

void fill_block(uint16_t tgt_low, uint16_t tgt_high, uint16_t c, uint16_t width);
#pragma aux fill_block = \
"shl bx, 3" \
"mov dx, 320" \
"sub dx, bx" \
"shr bx, 1" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
"add di, dx" \
"mov cx, bx" \
"rep stosw" \
parm [di] [es] [ax] [bx];

void hline(uint16_t tgt_low, uint16_t tgt_high, uint16_t c, uint16_t width);
#pragma aux hline = \
"shr cx, 1" \
"rep stosw" \
"adc cx, cx" \
"rep stosb" \
parm [di] [es] [ax] [cx];

//

static void tri(
    int16_t x0, int16_t y0,
    int16_t x1, int16_t y1,
    int16_t x2, int16_t y2,
    uint8_t color,
    uint8_t __far* screen)
{
    uint16_t min_x, min_y, max_x, max_y;
    int16_t dx0, dx1, dx2;
    int16_t dy0, dy1, dy2;
    int16_t eo0, eo1, eo2;
    int16_t co0_10, co0_01, co0_11;
    int16_t co1_10, co1_01, co1_11;
    int16_t co2_10, co2_01, co2_11;
    int16_t c0, c1, c2;
    int16_t cy0, cy1, cy2;
    uint16_t color16 = ((uint16_t)color << 8) | color;

    uint8_t __far* screen_row;
    uint8_t __far* screen_row_end;

    min_x = clamp16(min16(x0, min16(x1, x2)), 0, RASTER_SCREEN_X_MAX) & ~RASTER_BLOCK_MASK;
    min_y = clamp16(min16(y0, min16(y1, y2)), 0, RASTER_SCREEN_Y_MAX) & ~RASTER_BLOCK_MASK;
    max_x = clamp16(max16(x0, max16(x1, x2)), 0, RASTER_SCREEN_X_MAX);
    max_y = clamp16(max16(y0, max16(y1, y2)), 0, RASTER_SCREEN_Y_MAX);

    dx0 = x0 - x1;
    dx1 = x1 - x2;
    dx2 = x2 - x0;

    dy0 = y0 - y1;
    dy1 = y1 - y2;
    dy2 = y2 - y0;

    // Offsets for empty testing
    eo0 = 0;
    if (dy0 < 0) eo0 = eo0 - (dy0 << RASTER_BLOCK_SIZE_SHIFT);
    if (dx0 > 0) eo0 = eo0 + (dx0 << RASTER_BLOCK_SIZE_SHIFT);

    eo1 = 0;
    if (dy1 < 0) eo1 = eo1 - (dy1 << RASTER_BLOCK_SIZE_SHIFT);
    if (dx1 > 0) eo1 = eo1 + (dx1 << RASTER_BLOCK_SIZE_SHIFT);

    eo2 = 0;
    if (dy2 < 0) eo2 = eo2 - (dy2 << RASTER_BLOCK_SIZE_SHIFT);
    if (dx2 > 0) eo2 = eo2 + (dx2 << RASTER_BLOCK_SIZE_SHIFT);

    // Corner offsets
    co0_10 = -mul_by_raster_block_mask(dy0);
    co0_01 = mul_by_raster_block_mask(dx0);
    co0_11 = mul_by_raster_block_mask(dx0 - dy0);

    co1_10 = -mul_by_raster_block_mask(dy1);
    co1_01 = mul_by_raster_block_mask(dx1);
    co1_11 = mul_by_raster_block_mask(dx1 - dy1);

    co2_10 = -mul_by_raster_block_mask(dy2);
    co2_01 = mul_by_raster_block_mask(dx2);
    co2_11 = mul_by_raster_block_mask(dx2 - dy2);

    //

    c0 = dy0 * x0 - dx0 * y0;
    c1 = dy1 * x1 - dx1 * y1;
    c2 = dy2 * x2 - dx2 * y2;

    if (dy0 < 0 || (dy0 == 0 && dx0 > 0)) c0 += 1;
    if (dy1 < 0 || (dy1 == 0 && dx1 > 0)) c1 += 1;
    if (dy2 < 0 || (dy2 == 0 && dx2 > 0)) c2 += 1;

    cy0 = c0 + dx0 * min_y - dy0 * min_x;
    cy1 = c1 + dx1 * min_y - dy1 * min_x;
    cy2 = c2 + dx2 * min_y - dy2 * min_x;

    //

    screen_row = screen + mul_by_screen_stride(min_y);
    screen_row_end = screen + mul_by_screen_stride(max_y);

    while (screen_row < screen_row_end) {
        int16_t cx0 = cy0;
        int16_t cx1 = cy1;
        int16_t cx2 = cy2;

        uint16_t x = min_x;
        while (x < max_x && !((cx0 + eo0) > 0 &&
                              (cx1 + eo1) > 0 &&
                              (cx2 + eo2) > 0)) {
            cx0 -= dy0 << RASTER_BLOCK_SIZE_SHIFT;
            cx1 -= dy1 << RASTER_BLOCK_SIZE_SHIFT;
            cx2 -= dy2 << RASTER_BLOCK_SIZE_SHIFT;
            x += RASTER_BLOCK_SIZE;
        }

        while (x < max_x) {
            uint8_t __far* screen_block = screen_row + x;
            uint8_t num_completely_filled = 0;

            while (x < max_x &&
                   cx0 > 0 && (cx0 + co0_10) > 0 &&
                              (cx0 + co0_01) > 0 &&
                              (cx0 + co0_11) > 0 &&
                   cx1 > 0 && (cx1 + co1_10) > 0 &&
                              (cx1 + co1_01) > 0 &&
                              (cx1 + co1_11) > 0 &&
                   cx2 > 0 && (cx2 + co2_10) > 0 &&
                              (cx2 + co2_01) > 0 &&
                              (cx2 + co2_11) > 0) {
                num_completely_filled++;
                cx0 -= dy0 << RASTER_BLOCK_SIZE_SHIFT;
                cx1 -= dy1 << RASTER_BLOCK_SIZE_SHIFT;
                cx2 -= dy2 << RASTER_BLOCK_SIZE_SHIFT;
                x += RASTER_BLOCK_SIZE;
            }

            if (num_completely_filled != 0) {
#if 1
                fill_block((uint16_t)screen_block,
                    (uint16_t)((uint32_t)screen_block >> 16),
                    color16, num_completely_filled);
#else
                uint16_t width = num_completely_filled << RASTER_BLOCK_SIZE_SHIFT;
                uint8_t iy;
                for (iy = 0; iy < RASTER_BLOCK_SIZE; ++iy) {
                    uint16_t ix;
                    for (ix = 0; ix < width; ix++) {
                        screen_block[ix] = color;
                    }

                    screen_block += SCREEN_STRIDE;
                }
#endif
            } else {
                // Partially filled
                int16_t ciy0 = cx0;
                int16_t ciy1 = cx1;
                int16_t ciy2 = cx2;
                uint8_t iy;
                for (iy = 0; iy < RASTER_BLOCK_SIZE; ++iy) {
                    int16_t cix0 = ciy0;
                    int16_t cix1 = ciy1;
                    int16_t cix2 = ciy2;

                    uint8_t left = 0;
                    while ((cix0 | cix1 | cix2) <= 0) {
                        cix0 -= dy0;
                        cix1 -= dy1;
                        cix2 -= dy2;

                        ++left;
                        if (left == RASTER_BLOCK_SIZE) {
                            break;
                        }
                    }

                    if (left != RASTER_BLOCK_SIZE) {
                        uint8_t right = left;

                        while ((cix0 | cix1 | cix2) > 0) {
                            cix0 -= dy0;
                            cix1 -= dy1;
                            cix2 -= dy2;

                            right++;
                            if (right == RASTER_BLOCK_SIZE) {
                                break;
                            }
                        }

#if 1
                        {
                            uint8_t __far* tgt = screen_block + left;
                            hline((uint16_t)tgt,
                                (uint16_t)((uint32_t)tgt >> 16),
                                color16, right - left);
                        }
#else
                        {
                            uint8_t __far* h = screen_block + left;
                            uint8_t __far* h_end = screen_block + right;
                            while (h < h_end) {
                                *h++ = color;
                            }
                        }
#endif
                    }

                    ciy0 += dx0;
                    ciy1 += dx1;
                    ciy2 += dx2;

                    screen_block += SCREEN_STRIDE;
                }

                cx0 -= dy0 << RASTER_BLOCK_SIZE_SHIFT;
                cx1 -= dy1 << RASTER_BLOCK_SIZE_SHIFT;
                cx2 -= dy2 << RASTER_BLOCK_SIZE_SHIFT;

                x += RASTER_BLOCK_SIZE;
            }

            if ((cx0 + eo0) <= 0 || (cx1 + eo1) <= 0 || (cx2 + eo2) <= 0) {
                break;
            }
        }

        cy0 += dx0 << RASTER_BLOCK_SIZE_SHIFT;
        cy1 += dx1 << RASTER_BLOCK_SIZE_SHIFT;
        cy2 += dx2 << RASTER_BLOCK_SIZE_SHIFT;

        screen_row += SCREEN_STRIDE << RASTER_BLOCK_SIZE_SHIFT;
    }
}

#endif
