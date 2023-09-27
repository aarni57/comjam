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

//

static inline void tri(
    int16_t x0, int16_t y0,
    int16_t x1, int16_t y1,
    int16_t x2, int16_t y2,
    uint8_t color) {
    uint16_t min_x, min_y, max_x, max_y;
    fx_t dx0, dx1, dx2;
    fx_t dy0, dy1, dy2;
    fx_t eo0, eo1, eo2; // Offsets for empty testing
    fx_t c0, c1, c2;

    uint8_t __far* screen_row;
    uint8_t __far* screen_row_end;

    max_x = clamp16((max16(x0, max16(x1, x2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_X_MAX);
    if (max_x == 0)
        return;

    max_y = clamp16((max16(y0, max16(y1, y2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_Y_MAX);
    if (max_y == 0)
        return;

    min_x = clamp16((min16(x0, min16(x1, x2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_X_MAX) & ~RASTER_BLOCK_MASK;
    if (min_x == max_x)
        return;

    min_y = clamp16((min16(y0, min16(y1, y2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_Y_MAX) & ~RASTER_BLOCK_MASK;
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
        uint16_t x = min_x;
        fx_t cx0, cx1, cx2;

        cx0 = c0;
        cx1 = c1;
        cx2 = c2;

#if 1
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
#else
        // TODO: Not working
        __asm {
            sal dy0, 3
            sal dy1, 3
            sal dy2, 3

            mov dx, max_x

            contblock:
            mov eax, cx0
            mov ebx, eo0
            add eax, ebx
            cmp eax, 0
            jg doneblock

            mov eax, cx1
            mov ebx, eo1
            add eax, ebx
            cmp eax, 0
            jg doneblock

            mov eax, cx2
            mov ebx, eo2
            add eax, ebx
            cmp eax, 0
            jg doneblock

            mov eax, cx0
            sub eax, dy0
            mov cx0, eax

            mov eax, cx1
            sub eax, dy1
            mov cx1, eax

            mov eax, cx2
            sub eax, dy2
            mov cx2, eax

            mov ax, x
            add ax, 8
            mov x, ax
            cmp ax, dx
            jb contblock

            doneblock:
            sar dy0, 3
            sar dy1, 3
            sar dy2, 3
        }
#endif

        while (x < max_x) {
            uint8_t __far* screen_block = screen_row + x;

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
