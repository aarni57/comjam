#ifndef TRI_H
#define TRI_H

#include "minmax.h"
#include "screen.h"
#include "fx.h"

//

#define RASTER_SCREEN_X_MAX (SCREEN_WIDTH - 1)
#define RASTER_SCREEN_Y_MAX (SCREEN_HEIGHT - 1)

#define RASTER_BLOCK_SIZE_SHIFT 2
#define RASTER_BLOCK_SIZE       4
#define RASTER_BLOCK_MASK       3

#define RASTER_SUBPIXEL_BITS    4
#define RASTER_SUBPIXEL_ONE     16
#define RASTER_SUBPIXEL_MASK    15
#define RASTER_SUBPIXEL_HALF    8

//

static inline void tris(
    int16_t __far* draw_buffer,
    uint32_t __far* sort_buffer,
    uint16_t num) {
    uint16_t i, j;
    int16_t x0, y0, x1, y1, x2, y2;
    uint16_t min_x, min_y, max_x, max_y;
    fx_t dx0, dx1, dx2;
    fx_t dy0, dy1, dy2;
    fx_t eo0, eo1, eo2; // Offsets for empty testing
    fx_t c0, c1, c2, c;
    uint16_t color16;
    uint8_t color;

    uint8_t __far* screen_row;
    uint8_t __far* screen_row_end;

    for (i = 0; i < num; ++i) {
        j = sort_buffer[i] >> 16;
        aw_assert(j < num);
        j <<= 3;

        x0 = draw_buffer[j + 0];
        y0 = draw_buffer[j + 1];
        x1 = draw_buffer[j + 2];
        y1 = draw_buffer[j + 3];
        x2 = draw_buffer[j + 4];
        y2 = draw_buffer[j + 5];
        color = draw_buffer[j + 6];

        max_x = clamp16((max16(x0, max16(x1, x2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_X_MAX);
        if (max_x == 0)
            continue;

        max_y = clamp16((max16(y0, max16(y1, y2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_Y_MAX);
        if (max_y == 0)
            continue;

        min_x = clamp16((min16(x0, min16(x1, x2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_X_MAX) & ~RASTER_BLOCK_MASK;
        if (min_x == max_x)
            continue;

        min_y = clamp16((min16(y0, min16(y1, y2)) + RASTER_SUBPIXEL_MASK) >> RASTER_SUBPIXEL_BITS, 0, RASTER_SCREEN_Y_MAX) & ~RASTER_BLOCK_MASK;
        if (min_y == max_y)
            continue;

        //

        dx0 = x0 - x1;
        dy0 = y0 - y1;
        dx1 = x1 - x2;
        dy1 = y1 - y2;
        dx2 = x2 - x0;
        dy2 = y2 - y0;

        //

#if 0
        c = imul32(dy0, x0) - imul32(dx0, y0);
#else
        __asm {
            .386
            mov eax, dy0
            movsx ebx, x0
            imul ebx
            mov ecx, eax

            mov eax, dx0
            movsx ebx, y0
            imul ebx

            sub ecx, eax
            mov c, ecx
        }
#endif

        if (dy0 < 0 || (dy0 == 0 && dx0 > 0)) c++;

#if 0
        dx0 <<= RASTER_SUBPIXEL_BITS;
        dy0 <<= RASTER_SUBPIXEL_BITS;
        c0 = c + imul32(dx0, min_y) - imul32(dy0, min_x);
#else
        __asm {
            .386
            mov eax, dx0
            shl eax, RASTER_SUBPIXEL_BITS
            mov dx0, eax

            movzx ebx, min_y
            imul ebx
            mov ecx, eax

            mov eax, dy0
            shl eax, RASTER_SUBPIXEL_BITS
            mov dy0, eax

            movzx ebx, min_x
            imul ebx

            sub ecx, eax
            mov eax, c
            add eax, ecx
            mov c0, eax
        }
#endif

        //

#if 0
        c = imul32(dy1, x1) - imul32(dx1, y1);
#else
        __asm {
            .386
            mov eax, dy1
            movsx ebx, x1
            imul ebx
            mov ecx, eax

            mov eax, dx1
            movsx ebx, y1
            imul ebx

            sub ecx, eax
            mov c, ecx
        }
#endif

        if (dy1 < 0 || (dy1 == 0 && dx1 > 0)) c++;

#if 0
        dx1 <<= RASTER_SUBPIXEL_BITS;
        dy1 <<= RASTER_SUBPIXEL_BITS;
        c1 = c + imul32(dx1, min_y) - imul32(dy1, min_x);
#else
        __asm {
            .386
            mov eax, dx1
            shl eax, RASTER_SUBPIXEL_BITS
            mov dx1, eax

            movzx ebx, min_y
            imul ebx
            mov ecx, eax

            mov eax, dy1
            shl eax, RASTER_SUBPIXEL_BITS
            mov dy1, eax

            movzx ebx, min_x
            imul ebx

            sub ecx, eax
            mov eax, c
            add eax, ecx
            mov c1, eax
        }
#endif

        //

#if 0
        c = imul32(dy2, x2) - imul32(dx2, y2);
#else
        __asm {
            .386
            mov eax, dy2
            movsx ebx, x2
            imul ebx
            mov ecx, eax

            mov eax, dx2
            movsx ebx, y2
            imul ebx

            sub ecx, eax
            mov c, ecx
        }
#endif

        if (dy2 < 0 || (dy2 == 0 && dx2 > 0)) c++;

#if 0
        dx2 <<= RASTER_SUBPIXEL_BITS;
        dy2 <<= RASTER_SUBPIXEL_BITS;
        c2 = c + imul32(dx2, min_y) - imul32(dy2, min_x);
#else
        __asm {
            .386
            mov eax, dx2
            shl eax, RASTER_SUBPIXEL_BITS
            mov dx2, eax

            movzx ebx, min_y
            imul ebx
            mov ecx, eax

            mov eax, dy2
            shl eax, RASTER_SUBPIXEL_BITS
            mov dy2, eax

            movzx ebx, min_x
            imul ebx

            sub ecx, eax
            mov eax, c
            add eax, ecx
            mov c2, eax
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

        color16 = (uint16_t)color | ((uint16_t)color << 8);

        screen_row = dblbuf + mul_by_screen_stride(min_y);
        screen_row_end = dblbuf + mul_by_screen_stride(max_y);

        while (screen_row < screen_row_end) {
            uint16_t x = min_x;
            fx_t cx0, cx1, cx2;

#if 0
            cx0 = c0;
            cx1 = c1;
            cx2 = c2;

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
            __asm {
                .386
                mov eax, c0
                mov ebx, c1
                mov ecx, c2
                mov cx0, eax
                mov cx1, ebx
                mov cx2, ecx

                block_test:
                mov eax, cx0
                add eax, eo0
                cmp eax, 0
                jle block_empty

                mov eax, cx1
                add eax, eo1
                cmp eax, 0
                jle block_empty

                mov eax, cx2
                add eax, eo2
                cmp eax, 0
                jle block_empty

                jmp block_found

                block_empty:
                mov eax, dy0
                mov ebx, dy1
                mov ecx, dy2
                shl eax, RASTER_BLOCK_SIZE_SHIFT
                shl ebx, RASTER_BLOCK_SIZE_SHIFT
                shl ecx, RASTER_BLOCK_SIZE_SHIFT
                sub cx0, eax
                sub cx1, ebx
                sub cx2, ecx

                mov ax, x
                add ax, RASTER_BLOCK_SIZE
                mov x, ax
                cmp ax, max_x
                jb block_test

                block_found:
            }
#endif

            while (x < max_x) {
                fx_t ciy0, ciy1, ciy2;
                uint8_t iy;
                uint8_t __far* screen_block = screen_row + x;

#if 0
                fx_t ciy0 = cx0;
                fx_t ciy1 = cx1;
                fx_t ciy2 = cx2;
#else
                __asm {
                    .386
                    mov eax, cx0
                    mov ebx, cx1
                    mov ecx, cx2
                    mov ciy0, eax
                    mov ciy1, ebx
                    mov ciy2, ecx
                }
#endif

                for (iy = 0; iy < RASTER_BLOCK_SIZE; ++iy) {
                    uint8_t left = 0;
                    fx_t cix0, cix1, cix2;
#if 0
                    cix0 = ciy0;
                    cix1 = ciy1;
                    cix2 = ciy2;
#else
                    __asm {
                        .386
                        mov eax, ciy0
                        mov ebx, ciy1
                        mov ecx, ciy2
                        mov cix0, eax
                        mov cix1, ebx
                        mov cix2, ecx
                    }
#endif

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
                        cmp dl, RASTER_BLOCK_SIZE
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
                            cmp dl, RASTER_BLOCK_SIZE
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
                                mov ax, color16
                                shr cx, 1
                                rep stosw
                                adc cx, cx
                                rep stosb
                            }
#endif
                        }
                    }

#if 0
                    ciy0 += dx0;
                    ciy1 += dx1;
                    ciy2 += dx2;
#else
                    __asm {
                        .386
                        mov eax, ciy0
                        mov ebx, ciy1
                        mov ecx, ciy2

                        add eax, dx0
                        add ebx, dx1
                        add ecx, dx2

                        mov ciy0, eax
                        mov ciy1, ebx
                        mov ciy2, ecx
                    }
#endif

                    screen_block += SCREEN_STRIDE;
                }

#if 0
                cx0 -= dy0 << RASTER_BLOCK_SIZE_SHIFT;
                cx1 -= dy1 << RASTER_BLOCK_SIZE_SHIFT;
                cx2 -= dy2 << RASTER_BLOCK_SIZE_SHIFT;
#else
                __asm {
                    .386
                    mov eax, cx0
                    mov ebx, dy0
                    shl ebx, RASTER_BLOCK_SIZE_SHIFT
                    sub eax, ebx
                    mov cx0, eax

                    mov eax, cx1
                    mov ebx, dy1
                    shl ebx, RASTER_BLOCK_SIZE_SHIFT
                    sub eax, ebx
                    mov cx1, eax

                    mov eax, cx2
                    mov ebx, dy2
                    shl ebx, RASTER_BLOCK_SIZE_SHIFT
                    sub eax, ebx
                    mov cx2, eax
                }
#endif

                x += RASTER_BLOCK_SIZE;

                if ((cx0 + eo0) <= 0 || (cx1 + eo1) <= 0 || (cx2 + eo2) <= 0) {
                    break;
                }
            }

#if 0
            c0 += dx0 << RASTER_BLOCK_SIZE_SHIFT;
            c1 += dx1 << RASTER_BLOCK_SIZE_SHIFT;
            c2 += dx2 << RASTER_BLOCK_SIZE_SHIFT;
#else
            __asm {
                .386
                mov eax, c0
                mov ebx, dx0
                shl ebx, RASTER_BLOCK_SIZE_SHIFT
                add eax, ebx
                mov c0, eax

                mov eax, c1
                mov ebx, dx1
                shl ebx, RASTER_BLOCK_SIZE_SHIFT
                add eax, ebx
                mov c1, eax

                mov eax, c2
                mov ebx, dx2
                shl ebx, RASTER_BLOCK_SIZE_SHIFT
                add eax, ebx
                mov c2, eax
            }
#endif

            screen_row += SCREEN_STRIDE << RASTER_BLOCK_SIZE_SHIFT;
        }
    }
}

#endif
