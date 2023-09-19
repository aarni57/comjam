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

static inline fx2_t project_to_screen(fx_t x, fx_t y, fx_t z) {
    fx_t ooz;
    fx2_t r;

    r.x = x;
    r.y = y;

    r.x *= SCREEN_LOGICAL_HEIGHT;
    r.y *= SCREEN_HEIGHT;

    ooz = ((fx_t)(1 << RASTER_SUBPIXEL_BITS) << OOZ_BITS) / z;
    r.x = (r.x * ooz) >> OOZ_BITS;
    r.y = (r.y * ooz) >> OOZ_BITS;

    r.y = -r.y;

    r.x += RASTER_SCREEN_CENTER_X;
    r.y += RASTER_SCREEN_CENTER_Y;

    return r;
}

static inline void transform_vertex(fx3_t* v, const fx3x3_t* rotation,
    const fx3_t* translation) {

#if 0
    v->x = ((v->x * size->x) >> 7) + center->x;
    v->y = ((v->y * size->y) >> 7) + center->y;
    v->z = ((v->z * size->z) >> 7) + center->z;
#endif

    v->x = ((v->x * rotation->m[0] + v->y * rotation->m[1] + v->z * rotation->m[2]) >> 16) + translation->x;
    v->y = ((v->x * rotation->m[3] + v->y * rotation->m[4] + v->z * rotation->m[5]) >> 16) + translation->y;
    v->z = ((v->x * rotation->m[6] + v->y * rotation->m[7] + v->z * rotation->m[8]) >> 16) + translation->z;
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

static void draw_mesh(uint16_t num_indices, uint16_t num_vertices,
    const fx3x3_t* model_view_rotation, const fx3_t* model_view_translation,
    const uint8_t* indices, const int8_t* vertices) {

    if (num_triangles == MAX_TRIANGLES) {
        return;
    }

    {
        uint16_t i = 0, j;
        uint16_t j_end = num_vertices * 3;

        for (j = 0; j < j_end; j += 3) {
            fx3_t p;
            fx2_t v;

            p.x = vertices[j + 0];
            p.y = vertices[j + 1];
            p.z = vertices[j + 2];

            transform_vertex(&p, model_view_rotation, model_view_translation);

            v = project_to_screen(p.x, p.y, p.z);
            tm_buffer[i++] = v.x;
            tm_buffer[i++] = v.y;
            tm_buffer[i++] = p.z;
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
                uint8_t c = 7; // TODO

                *draw_buffer_tgt++ = x0;
                *draw_buffer_tgt++ = y0;
                *draw_buffer_tgt++ = x1;
                *draw_buffer_tgt++ = y1;
                *draw_buffer_tgt++ = x2;
                *draw_buffer_tgt++ = y2;
                *draw_buffer_tgt++ = c;
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

static void flush_draw_buffer() {
    int16_t x0, y0, x1, y1, x2, y2;
    uint8_t c;
    uint16_t i, count;

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
        c = draw_buffer[j + 6];

        draw_tri(x0, y0, x1, y1, x2, y2, 6);
        draw_triangle_lines(x0, y0, x1, y1, x2, y2, 7);
     }

     num_triangles = 0;
}

#endif
