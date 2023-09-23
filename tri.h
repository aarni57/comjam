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
#define RASTER_SUBPIXEL_HALF (RASTER_SUBPIXEL_ONE >> 1)

#define RASTER_SCREEN_CENTER_X ((fx_t)SCREEN_CENTER_X << RASTER_SUBPIXEL_BITS)
#define RASTER_SCREEN_CENTER_Y ((fx_t)SCREEN_CENTER_Y << RASTER_SUBPIXEL_BITS)

//

#if 0
static inline fx_t mul_by_raster_block_mask(fx_t x) {
    return (x << 3) - x; // x * RASTER_BLOCK_MASK
}
#endif

//

static inline void tri(
    fx_t x0, fx_t y0,
    fx_t x1, fx_t y1,
    fx_t x2, fx_t y2,
    uint8_t color)
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
        fx_t c = imul32(dy0, x0) - imul32(dx0, y0);
        if (dy0 < 0 || (dy0 == 0 && dx0 > 0)) c++;

        dx0 <<= RASTER_SUBPIXEL_BITS;
        dy0 <<= RASTER_SUBPIXEL_BITS;

        c0 = c + imul32(dx0, min_y) - imul32(dy0, min_x);
    }
    {
        fx_t c = imul32(dy1, x1) - imul32(dx1, y1);
        if (dy1 < 0 || (dy1 == 0 && dx1 > 0)) c++;

        dx1 <<= RASTER_SUBPIXEL_BITS;
        dy1 <<= RASTER_SUBPIXEL_BITS;

        c1 = c + imul32(dx1, min_y) - imul32(dy1, min_x);
    }
    {
        fx_t c = imul32(dy2, x2) - imul32(dx2, y2);
        if (dy2 < 0 || (dy2 == 0 && dx2 > 0)) c++;

        dx2 <<= RASTER_SUBPIXEL_BITS;
        dy2 <<= RASTER_SUBPIXEL_BITS;

        c2 = c + imul32(dx2, min_y) - imul32(dy2, min_x);
    }

    //

#if 0
    co0_10 = mul_by_raster_block_mask(-dy0);
    co0_01 = mul_by_raster_block_mask(dx0);
    co0_11 = mul_by_raster_block_mask(dx0 - dy0);
    co1_10 = mul_by_raster_block_mask(-dy1);
    co1_01 = mul_by_raster_block_mask(dx1);
    co1_11 = mul_by_raster_block_mask(dx1 - dy1);
    co2_10 = mul_by_raster_block_mask(-dy2);
    co2_01 = mul_by_raster_block_mask(dx2);
    co2_11 = mul_by_raster_block_mask(dx2 - dy2);
#else

    __asm {
        .386

        // 0
        mov eax, dy0
        neg eax
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co0_10, eax

        mov eax, dx0
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co0_01, eax

        mov eax, dx0
        mov ebx, dy0
        sub eax, ebx
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co0_11, eax

        // 1
        mov eax, dy1
        neg eax
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co1_10, eax

        mov eax, dx1
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co1_01, eax

        mov eax, dx1
        mov ebx, dy1
        sub eax, ebx
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co1_11, eax

        // 2
        mov eax, dy2
        neg eax
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co2_10, eax

        mov eax, dx2
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co2_01, eax

        mov eax, dx2
        mov ebx, dy2
        sub eax, ebx
        mov ebx, eax
        sal eax, 3
        sub eax, ebx
        mov co2_11, eax
    }

#endif

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

    screen_row = dblbuf + mul_by_screen_stride(min_y);
    screen_row_end = dblbuf + mul_by_screen_stride(max_y);

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
#if 0
                uint8_t iy;
                uint8_t width = num_completely_filled << RASTER_BLOCK_SIZE_SHIFT;
                for (iy = 0; iy < RASTER_BLOCK_SIZE; ++iy) {
                    uint16_t ix;
                    for (ix = 0; ix < width; ix++) {
                        screen_block[ix] = color;
                    }

                    screen_block += SCREEN_STRIDE;
                }
#else
                __asm {
                    .386
                    mov bl, num_completely_filled
                    shl bl, 1
                    mov bh, 8

                    mov al, color
                    mov ah, al
                    shl eax, 8
                    mov al, ah
                    shl eax, 8
                    mov al, ah

                    mov edx, screen_block
                    ror edx, 16
                    mov es, dx
                    rol edx, 16

                    vl:
                    movzx edi, dx

                    movzx cx, bl
                    rep stosd

                    dec bh
                    jz done

                    add dx, 320

                    jmp vl
                    done:
                }
#endif
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

#if 0
                    while ((cix0 | cix1 | cix2) <= 0) {
                        cix0 -= dy0;
                        cix1 -= dy1;
                        cix2 -= dy2;

                        ++left;
                        if (left == RASTER_BLOCK_SIZE) {
                            break;
                        }
                    }
#else
                    __asm {
                        .386
                        mov eax, cix0
                        mov ebx, cix1
                        mov ecx, cix2

                        cont:
                        mov edx, eax
                        or edx, ebx
                        or edx, ecx
                        cmp edx, 0
                        jg done

                        mov edx, dy0
                        sub eax, edx

                        mov edx, dy1
                        sub ebx, edx

                        mov edx, dy2
                        sub ecx, edx

                        mov dl, left
                        inc dl
                        mov left, dl
                        cmp dl, 8
                        je done

                        jmp cont
                        done:
                        mov cix0, eax
                        mov cix1, ebx
                        mov cix2, ecx
                    }
#endif

                    if (left != RASTER_BLOCK_SIZE) {
                        uint8_t right = left;

#if 0
                        while ((cix0 | cix1 | cix2) > 0) {
                            cix0 -= dy0;
                            cix1 -= dy1;
                            cix2 -= dy2;

                            right++;
                            if (right == RASTER_BLOCK_SIZE) {
                                break;
                            }
                        }
#else
                        __asm {
                            .386
                            mov eax, cix0
                            mov ebx, cix1
                            mov ecx, cix2

                            cont:
                            mov edx, eax
                            or edx, ebx
                            or edx, ecx
                            cmp edx, 0
                            jle done

                            mov edx, dy0
                            sub eax, edx

                            mov edx, dy1
                            sub ebx, edx

                            mov edx, dy2
                            sub ecx, edx

                            mov dl, right
                            inc dl
                            mov right, dl
                            cmp dl, 8
                            je done

                            jmp cont
                            done:
                        }
#endif

                        {
#if 0
                            uint8_t __far* tgt = screen_block + left;
                            uint16_t width = right - left;
                            uint8_t __far* tgt_end = tgt + width;
                            while (tgt < tgt_end) {
                                *tgt++ = color;
                            }
#else
                            __asm {
                                .386
                                mov edx, screen_block
                                movzx eax, left
                                add edx, eax
                                movzx cx, right
                                sub cx, ax
                                movzx edi, dx
                                shr edx, 16
                                mov es, dx
                                mov al, color
                                mov ah, al
                                shr cx, 1
                                rep stosw
                                adc cx, cx
                                rep stosb
                            }
#endif
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

#if 0
#define TRI_SPLITTING_THRESHOLD 0xffff8000

static inline void draw_tri(
    fx_t x0, fx_t y0,
    fx_t x1, fx_t y1,
    fx_t x2, fx_t y2,
    uint8_t c) {
    if ((fx_abs(x0 - x1) | fx_abs(y0 - y1)) & TRI_SPLITTING_THRESHOLD) {
        fx_t sx = (x0 + x1) / 2;
        fx_t sy = (y0 + y1) / 2;
        draw_tri(x2, y2, x0, y0, sx, sy, c);
        draw_tri(x1, y1, x2, y2, sx, sy, c);
        return;
    }

    if ((fx_abs(x1 - x2) | fx_abs(y1 - y2)) & TRI_SPLITTING_THRESHOLD) {
        fx_t sx = (x1 + x2) / 2;
        fx_t sy = (y1 + y2) / 2;
        draw_tri(x0, y0, x1, y1, sx, sy, c);
        draw_tri(x2, y2, x0, y0, sx, sy, c);
        return;
    }

    if ((fx_abs(x2 - x0) | fx_abs(y2 - y0)) & TRI_SPLITTING_THRESHOLD) {
        fx_t sx = (x0 + x2) / 2;
        fx_t sy = (y0 + y2) / 2;
        draw_tri(x1, y1, x2, y2, sx, sy, c);
        draw_tri(x0, y0, x1, y1, sx, sy, c);
        return;
    }

    tri(x0, y0, x1, y1, x2, y2, c);
}
#else
#endif

#endif
