#ifndef DRAW_H
#define DRAW_H

#include "sort.h"
#include "tri.h"
#include "line.h"
#include "fx.h"

//

#define NEAR_CLIP 32

static inline void project_to_screen(fx3_t* v) {
#if 0
    fx_t ooz;

    if (v->z < NEAR_CLIP) {
        return;
    }

    fx_t ooz = 0xffffffff / v->z;

    v->x *= SCREEN_LOGICAL_HEIGHT;
    v->y *= SCREEN_HEIGHT;

    v->x = fx_mul(v->x, ooz) >> (16 - RASTER_SUBPIXEL_BITS);
    v->y = fx_mul(v->y, -ooz) >> (16 - RASTER_SUBPIXEL_BITS);

    v->x += RASTER_SCREEN_CENTER_X;
    v->y += RASTER_SCREEN_CENTER_Y;
#else
    if (v->z < NEAR_CLIP) {
        return;
    }

    __asm {
        .386
        mov si, v

        mov ecx, 8[si]
        mov edx, 1
        xor eax, eax
        idiv ecx
        mov ebx, eax

        // x
        mov eax, [si]
        mov ecx, 240
        imul ecx

        imul ebx
        sar eax, 16
        sal edx, 16
        mov dx, ax

        sar edx, 12
        add edx, 2560
        mov [si], edx

        // y
        mov eax, 4[si]
        mov ecx, 200
        imul ecx

        imul ebx
        sar eax, 16
        sal edx, 16
        mov dx, ax

        sar edx, 12
        neg edx
        add edx, 1600
        mov 4[si], edx
    }
#endif
}

//

static inline void draw_triangle_lines(fx_t x0, fx_t y0, fx_t x1, fx_t y1, fx_t x2, fx_t y2, uint8_t c) {
    x0 = (x0 + RASTER_SUBPIXEL_HALF) >> RASTER_SUBPIXEL_BITS;
    y0 = (y0 + RASTER_SUBPIXEL_HALF) >> RASTER_SUBPIXEL_BITS;
    x1 = (x1 + RASTER_SUBPIXEL_HALF) >> RASTER_SUBPIXEL_BITS;
    y1 = (y1 + RASTER_SUBPIXEL_HALF) >> RASTER_SUBPIXEL_BITS;
    x2 = (x2 + RASTER_SUBPIXEL_HALF) >> RASTER_SUBPIXEL_BITS;
    y2 = (y2 + RASTER_SUBPIXEL_HALF) >> RASTER_SUBPIXEL_BITS;
    draw_line(x0, y0, x1, y1, c);
    draw_line(x1, y1, x2, y2, c);
    draw_line(x2, y2, x0, y0, c);
}

static inline int32_t calc_triangle_area(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
#if 0
    return imul32(x2 - x0, y1 - y0) - imul32(y2 - y0, x1 - x0);
#else
    int32_t a;

    __asm {
        // a = (x2 - x0) * (y1 - y0)
        movsx eax, x2
        movsx ebx, x0
        sub eax, ebx

        movsx ebx, y1
        movsx ecx, y0
        sub ebx, ecx

        imul ebx
        mov a, eax

        // b = (y2 - y0) * (x1 - x0)
        movsx eax, y2
        movsx ebx, y0
        sub eax, ebx

        movsx ebx, x1
        movsx ecx, x0
        sub ebx, ecx

        imul ebx

        // a - b
        mov ebx, a
        sub ebx, eax
        mov a, ebx
    }

    return a;
#endif
}

//

#define TM_BUFFER_COUNT 1024
#define TM_BUFFER_SIZE (TM_BUFFER_COUNT * 3)
static int16_t __far* tm_buffer = NULL;

#define DRAW_BUFFER_MAX_TRIANGLES 1024
static uint16_t draw_buffer_num_triangles = 0;
static int16_t __far* draw_buffer = NULL;
static uint32_t __far* sort_buffer = NULL;

static void draw_mesh(const fx4x3_t* model_view_matrix,
    uint16_t num_indices,
    uint16_t num_vertices,
    const uint16_t* indices,
    const uint8_t* face_colors,
    const int8_t* vertices) {

    aw_assert(num_vertices <= TM_BUFFER_COUNT);
    aw_assert(num_indices % 3 == 0);
    aw_assert(draw_buffer_num_triangles + num_indices / 3 <= DRAW_BUFFER_MAX_TRIANGLES);

    {
        const int8_t* vertices_end = vertices + ((num_vertices << 1) + num_vertices);
        int16_t __far* tm_iter = tm_buffer;

        while (vertices < vertices_end) {
            fx3_t v;
            v.x = *vertices++;
            v.y = *vertices++;
            v.z = *vertices++;

            fx_transform_point_ip(model_view_matrix, &v);

            project_to_screen(&v);
            *tm_iter++ = fx_clamp(v.x, INT16_MIN, INT16_MAX);
            *tm_iter++ = fx_clamp(v.y, INT16_MIN, INT16_MAX);
            *tm_iter++ = fx_clamp(v.z, INT16_MIN, INT16_MAX);
        }
    }

    {
        int16_t __far* draw_buffer_tgt = draw_buffer + (draw_buffer_num_triangles << 3);
        uint16_t __far* sort_buffer_tgt = (uint16_t __far*)(sort_buffer + draw_buffer_num_triangles);
        const uint16_t* indices_end = indices + num_indices;
        uint16_t a, b, c;
        uint8_t face_color;
        int16_t x0, y0, z0, x1, y1, z1, x2, y2, z2;

        while (indices < indices_end) {
            a = *indices++;
            b = *indices++;
            c = *indices++;
            face_color = *face_colors++;

            a = (a << 1) + a;
            z0 = tm_buffer[a + 2];
            if (z0 < NEAR_CLIP)
                continue;

            x0 = tm_buffer[a + 0];
            y0 = tm_buffer[a + 1];

            b = (b << 1) + b;
            z2 = tm_buffer[b + 2];
            if (z2 < NEAR_CLIP)
                continue;

            x2 = tm_buffer[b + 0];
            y2 = tm_buffer[b + 1];

            c = (c << 1) + c;
            z1 = tm_buffer[c + 2];
            if (z1 < NEAR_CLIP)
                continue;

            x1 = tm_buffer[c + 0];
            y1 = tm_buffer[c + 1];

            if (calc_triangle_area(x0, y0, x1, y1, x2, y2) > 0) {
                uint32_t z_value = ((uint32_t)z0 + z1 + z2) >> 4;
                aw_assert(z_value < 0x10000UL);

                draw_buffer_tgt[0] = x0;
                draw_buffer_tgt[1] = y0;
                draw_buffer_tgt[2] = x1;
                draw_buffer_tgt[3] = y1;
                draw_buffer_tgt[4] = x2;
                draw_buffer_tgt[5] = y2;
                draw_buffer_tgt[6] = face_color;
                draw_buffer_tgt += 8;

                sort_buffer_tgt[0] = z_value;
                sort_buffer_tgt[1] = draw_buffer_num_triangles;
                sort_buffer_tgt += 2;

                draw_buffer_num_triangles++;
            }
        }
    }
}

static void flush_mesh_draw_buffer(int draw_mode) {
    int16_t x0, y0, x1, y1, x2, y2;
    uint8_t color;
    uint16_t i;

    aw_assert(draw_buffer_num_triangles <= DRAW_BUFFER_MAX_TRIANGLES);
    smoothsort(sort_buffer, draw_buffer_num_triangles);

    switch (draw_mode) {
    case 0:
        for (i = 0; i < draw_buffer_num_triangles; ++i) {
            uint16_t j = sort_buffer[i] >> 16;
            aw_assert(j < DRAW_BUFFER_MAX_TRIANGLES);
            j <<= 3;

            x0 = draw_buffer[j + 0];
            y0 = draw_buffer[j + 1];
            x1 = draw_buffer[j + 2];
            y1 = draw_buffer[j + 3];
            x2 = draw_buffer[j + 4];
            y2 = draw_buffer[j + 5];
            color = draw_buffer[j + 6];

            tri(x0, y0, x1, y1, x2, y2, color);
            //draw_triangle_lines(x0, y0, x1, y1, x2, y2, color + 1);
        }

        break;

    case 1:
        for (i = 0; i < draw_buffer_num_triangles; ++i) {
            uint16_t j = sort_buffer[i] >> 16;
            aw_assert(j < DRAW_BUFFER_MAX_TRIANGLES);
            j <<= 3;

            x0 = draw_buffer[j + 0];
            y0 = draw_buffer[j + 1];
            x1 = draw_buffer[j + 2];
            y1 = draw_buffer[j + 3];
            x2 = draw_buffer[j + 4];
            y2 = draw_buffer[j + 5];
            color = draw_buffer[j + 6];

            draw_triangle_lines(x0, y0, x1, y1, x2, y2, color);
        }

        break;

    default:
        break;
    }

     draw_buffer_num_triangles = 0;
}

#endif
