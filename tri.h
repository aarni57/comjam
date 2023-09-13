#ifndef TRI_H
#define TRI_H

#include "minmax.h"
#include "screen.h"
#include "fx.h"

//

#define RASTER_SCREEN_X_MAX (SCREEN_WIDTH - 1)
#define RASTER_SCREEN_Y_MAX (SCREEN_HEIGHT - 1)

#define RASTER_BLOCK_SIZE_SHIFT 3
#define RASTER_BLOCK_SIZE (1 << RASTER_BLOCK_SIZE_SHIFT)
#define RASTER_BLOCK_MASK (RASTER_BLOCK_SIZE - 1)

#define RASTER_SUBPIXEL_BITS 4
#define RASTER_SUBPIXEL_ONE (1 << RASTER_SUBPIXEL_BITS)
#define RASTER_SUBPIXEL_MASK (RASTER_SUBPIXEL_ONE - 1)

#define RASTER_SCREEN_CENTER_X ((fx_t)SCREEN_CENTER_X << RASTER_SUBPIXEL_BITS)
#define RASTER_SCREEN_CENTER_Y ((fx_t)SCREEN_CENTER_Y << RASTER_SUBPIXEL_BITS)

//

static inline fx_t mul_by_raster_block_mask(fx_t x) {
    return (x << 3) - x; // x * RASTER_BLOCK_MASK
}

#if 1

#define PASS_FAR_PTR(ptr) (uint16_t)(ptr), (uint16_t)((uint32_t)(ptr) >> 16)

void fill_block(uint16_t tgt_low, uint16_t tgt_high, uint8_t c, uint16_t width);

#pragma aux fill_block = \
"mov ah, al" \
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
modify [cx dx] \
parm [di] [es] [al] [bx];

void hline(uint16_t tgt_low, uint16_t tgt_high, uint8_t c, uint16_t width);
#pragma aux hline = \
"mov ah, al" \
"shr cx, 1" \
"rep stosw" \
"adc cx, cx" \
"rep stosb" \
modify [ah] \
parm [di] [es] [al] [cx];

#else

#define PASS_FAR_PTR(ptr) ptr

void fill_block(uint8_t __far* tgt, uint8_t c, uint16_t width) {
    uint8_t iy;
    width <<= RASTER_BLOCK_SIZE_SHIFT;
    for (iy = 0; iy < RASTER_BLOCK_SIZE; ++iy) {
        uint16_t ix;
        for (ix = 0; ix < width; ix++) {
            tgt[ix] = c;
        }

        tgt += SCREEN_STRIDE;
    }
}

void hline(uint8_t __far* tgt, uint8_t c, uint16_t width) {
    uint8_t __far* tgt_end = tgt + width;
    while (tgt < tgt_end) {
        *tgt++ = c;
    }
}

#endif

//

static void tri(
    fx_t x0, fx_t y0,
    fx_t x1, fx_t y1,
    fx_t x2, fx_t y2,
    uint8_t color,
    uint8_t __far* screen)
{
    uint16_t min_x, min_y, max_x, max_y;
    fx_t dx0, dx1, dx2;
    fx_t dy0, dy1, dy2;
    fx_t eo0, eo1, eo2; // Offsets for empty testing
    fx_t co0_10, co0_01, co0_11; // Corner offsets
    fx_t co1_10, co1_01, co1_11;
    fx_t co2_10, co2_01, co2_11;
    fx_t c0, c1, c2;

    uint8_t __far* screen_row;
    uint8_t __far* screen_row_end;

    min_x = fx_clamp((fx_min(x0, fx_min(x1, x2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_X_MAX) & ~RASTER_BLOCK_MASK;
    max_x = fx_clamp((fx_max(x0, fx_max(x1, x2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_X_MAX);
    if (min_x == max_x)
        return;

    min_y = fx_clamp((fx_min(y0, fx_min(y1, y2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_Y_MAX) & ~RASTER_BLOCK_MASK;
    max_y = fx_clamp((fx_max(y0, fx_max(y1, y2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_Y_MAX);
    if (min_y == max_y)
        return;

    //

    dx0 = x0 - x1;
    dy0 = y0 - y1;
    dx1 = x1 - x2;
    dy1 = y1 - y2;
    dx2 = x2 - x0;
    dy2 = y2 - y0;

    {
        fx_t c = dy0 * x0 - dx0 * y0;
        if (dy0 < 0 || (dy0 == 0 && dx0 > 0)) c++;

        dx0 <<= RASTER_SUBPIXEL_BITS;
        dy0 <<= RASTER_SUBPIXEL_BITS;

        c0 = c + dx0 * min_y - dy0 * min_x;
    }
    {
        fx_t c = dy1 * x1 - dx1 * y1;
        if (dy1 < 0 || (dy1 == 0 && dx1 > 0)) c++;

        dx1 <<= RASTER_SUBPIXEL_BITS;
        dy1 <<= RASTER_SUBPIXEL_BITS;

        c1 = c + dx1 * min_y - dy1 * min_x;
    }
    {
        fx_t c = dy2 * x2 - dx2 * y2;
        if (dy2 < 0 || (dy2 == 0 && dx2 > 0)) c++;

        dx2 <<= RASTER_SUBPIXEL_BITS;
        dy2 <<= RASTER_SUBPIXEL_BITS;

        c2 = c + dx2 * min_y - dy2 * min_x;
    }

    //

    co0_10 = mul_by_raster_block_mask(-dy0);
    co0_01 = mul_by_raster_block_mask(dx0);
    co0_11 = mul_by_raster_block_mask(dx0 - dy0);
    co1_10 = mul_by_raster_block_mask(-dy1);
    co1_01 = mul_by_raster_block_mask(dx1);
    co1_11 = mul_by_raster_block_mask(dx1 - dy1);
    co2_10 = mul_by_raster_block_mask(-dy2);
    co2_01 = mul_by_raster_block_mask(dx2);
    co2_11 = mul_by_raster_block_mask(dx2 - dy2);

    //

    eo0 = 0;
    if (dy0 < 0) eo0 = eo0 - (dy0 << RASTER_BLOCK_SIZE_SHIFT);
    if (dx0 > 0) eo0 = eo0 + (dx0 << RASTER_BLOCK_SIZE_SHIFT);

    eo1 = 0;
    if (dy1 < 0) eo1 = eo1 - (dy1 << RASTER_BLOCK_SIZE_SHIFT);
    if (dx1 > 0) eo1 = eo1 + (dx1 << RASTER_BLOCK_SIZE_SHIFT);

    eo2 = 0;
    if (dy2 < 0) eo2 = eo2 - (dy2 << RASTER_BLOCK_SIZE_SHIFT);
    if (dx2 > 0) eo2 = eo2 + (dx2 << RASTER_BLOCK_SIZE_SHIFT);

    //

    screen_row = screen + mul_by_screen_stride(min_y);
    screen_row_end = screen + mul_by_screen_stride(max_y);

    while (screen_row < screen_row_end) {
        fx_t cx0 = c0;
        fx_t cx1 = c1;
        fx_t cx2 = c2;

        uint16_t x = min_x;
        while (!((cx0 + eo0) > 0 &&
                 (cx1 + eo1) > 0 &&
                 (cx2 + eo2) > 0)) {
            cx0 -= dy0 << RASTER_BLOCK_SIZE_SHIFT;
            cx1 -= dy1 << RASTER_BLOCK_SIZE_SHIFT;
            cx2 -= dy2 << RASTER_BLOCK_SIZE_SHIFT;
            x += RASTER_BLOCK_SIZE;
            if (x >= max_x)
                break;
        }

        while (x < max_x) {
            uint8_t __far* screen_block = screen_row + x;
            uint8_t num_completely_filled = 0;

            while (cx0 > 0 && (cx0 + co0_10) > 0 &&
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
                if (x >= max_x)
                    break;
            }

            if (num_completely_filled != 0) {
                fill_block(PASS_FAR_PTR(screen_block), color, num_completely_filled);
            } else {
                // Partially filled
                fx_t ciy0 = cx0;
                fx_t ciy1 = cx1;
                fx_t ciy2 = cx2;
                uint8_t iy;
                for (iy = 0; iy < RASTER_BLOCK_SIZE; ++iy) {
                    fx_t cix0 = ciy0;
                    fx_t cix1 = ciy1;
                    fx_t cix2 = ciy2;

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

                        {
                            uint8_t __far* tgt = screen_block + left;
                            hline(PASS_FAR_PTR(tgt), color, right - left);
                        }
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

        c0 += dx0 << RASTER_BLOCK_SIZE_SHIFT;
        c1 += dx1 << RASTER_BLOCK_SIZE_SHIFT;
        c2 += dx2 << RASTER_BLOCK_SIZE_SHIFT;

        screen_row += SCREEN_STRIDE << RASTER_BLOCK_SIZE_SHIFT;
    }
}

#endif
