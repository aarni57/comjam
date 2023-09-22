#ifndef DRAW_H
#define DRAW_H

#include "sort.h"
#include "tri.h"
#include "line.h"
#include "fx.h"

//

#define NEAR_CLIP 256
#define FAR_CLIP ((fx_t)1024 * 256)

#define OOZ_BITS 16

static inline void project_to_screen(fx3_t* v) {
    fx_t ooz = (1L << (RASTER_SUBPIXEL_BITS + OOZ_BITS)) / v->z;

    v->x *= SCREEN_LOGICAL_HEIGHT;
    v->y *= SCREEN_HEIGHT;

    v->x = (v->x * ooz) >> OOZ_BITS;
    v->y = (v->y * -ooz) >> OOZ_BITS;

    v->x += RASTER_SCREEN_CENTER_X;
    v->y += RASTER_SCREEN_CENTER_Y;
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

static inline fx_t calc_triangle_area(fx_t x0, fx_t y0, fx_t x1, fx_t y1, fx_t x2, fx_t y2) {
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}

//

#define TM_BUFFER_SIZE (512 * 3)
static fx_t __far* tm_buffer = NULL;

#define MAX_TRIANGLES 512
static uint16_t num_triangles = 0;
static int16_t __far* draw_buffer = NULL;
static uint32_t __far* sort_buffer = NULL;

static void draw_mesh(const fx4x3_t* model_view_matrix, uint8_t color,
    uint16_t num_indices, uint16_t num_vertices,
    const uint8_t* indices, const int8_t* vertices) {

    if (num_triangles == MAX_TRIANGLES) {
        return;
    }

    {
        uint16_t i = 0, j;
        uint16_t j_end = num_vertices * 3;

        for (j = 0; j < j_end; j += 3) {
            fx3_t p, v;

            p.x = vertices[j + 0];
            p.y = vertices[j + 1];
            p.z = vertices[j + 2];

            fx_transform_point(&v, model_view_matrix, &p);

            project_to_screen(&v);
            tm_buffer[i++] = v.x;
            tm_buffer[i++] = v.y;
            tm_buffer[i++] = v.z;
        }
    }

    {
        int16_t __far* draw_buffer_tgt = draw_buffer + (num_triangles << 3);
        uint32_t __far* sort_buffer_tgt = sort_buffer + num_triangles;
        const uint8_t* indices_end = indices + num_indices;
        uint16_t a, b, c;
        fx_t x0, y0, z0, x1, y1, z1, x2, y2, z2;

        while (indices < indices_end) {
            a = *indices++;
            b = *indices++;
            c = *indices++;

            a *= 3;
            z0 = tm_buffer[a + 2];
            if (z0 < NEAR_CLIP || z0 > FAR_CLIP)
                continue;

            x0 = tm_buffer[a + 0];
            y0 = tm_buffer[a + 1];

            b *= 3;
            z2 = tm_buffer[b + 2];
            if (z2 < NEAR_CLIP || z2 > FAR_CLIP)
                continue;

            x2 = tm_buffer[b + 0];
            y2 = tm_buffer[b + 1];

            c *= 3;
            z1 = tm_buffer[c + 2];
            if (z1 < NEAR_CLIP || z1 > FAR_CLIP)
                continue;

            x1 = tm_buffer[c + 0];
            y1 = tm_buffer[c + 1];

            if (calc_triangle_area(x0, y0, x1, y1, x2, y2) > 0) {
                *draw_buffer_tgt++ = x0;
                *draw_buffer_tgt++ = y0;
                *draw_buffer_tgt++ = x1;
                *draw_buffer_tgt++ = y1;
                *draw_buffer_tgt++ = x2;
                *draw_buffer_tgt++ = y2;
                *draw_buffer_tgt++ = color;
                *draw_buffer_tgt++ = 0; // Reserved

                *sort_buffer_tgt++ = ((z0 + z1 + z2) >> 4) | ((uint32_t)num_triangles << 16);

                num_triangles++;
                if (num_triangles == MAX_TRIANGLES) {
                    return;
                }
            }
        }
    }
}

static void flush_mesh_draw_buffer() {
    int16_t x0, y0, x1, y1, x2, y2;
    uint8_t color;
    uint16_t i;

    smoothsort(sort_buffer, num_triangles);

    for (i = 0; i < num_triangles; ++i) {
        uint16_t j = sort_buffer[i] >> 16;
        j <<= 3;

        x0 = draw_buffer[j + 0];
        y0 = draw_buffer[j + 1];
        x1 = draw_buffer[j + 2];
        y1 = draw_buffer[j + 3];
        x2 = draw_buffer[j + 4];
        y2 = draw_buffer[j + 5];
        color = draw_buffer[j + 6];

        draw_tri(x0, y0, x1, y1, x2, y2, color);
        //draw_triangle_lines(x0, y0, x1, y1, x2, y2, color + 1);
     }

     num_triangles = 0;
}

#endif
