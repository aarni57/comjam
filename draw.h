#ifndef DRAW_H
#define DRAW_H

#include "tri.h"
#include "line.h"
#include "fx.h"

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

static inline void transform_vertex(fx3_t* v, const fx3_t* size, const fx3_t* center) {
    v->x = ((v->x * size->x) >> 7) + center->x;
    v->y = ((v->y * size->y) >> 7) + center->y;
    v->z = ((v->z * size->z) >> 7) + center->z;
}

#define DRAW_BUFFER_SIZE 4096

static uint16_t draw_buffer_position = 0;
static int16_t __far* draw_buffer = NULL;
static uint32_t __far* sort_buffer = NULL;

static void draw_mesh(uint16_t num_indices, uint16_t num_vertices,
    const fx3_t* center, const fx3_t* size, fx_t rotation,
    const uint8_t* indices, const int8_t* vertices) {
    static int16_t tm_buffer[256 * 2];

    if (draw_buffer_position + num_indices / 3 * 8 > DRAW_BUFFER_SIZE) {
        return;
    }

    {
        uint16_t i = 0, j;
        uint16_t j_end = num_vertices * 3;
        fx_t c, s;
        fx_t t2;

        c = fx_cos(rotation);
        s = fx_sin(rotation);

        for (j = 0; j < j_end; j += 3) {
            fx3_t p;

            p.x = vertices[j + 0];
            p.y = vertices[j + 1];
            p.z = vertices[j + 2];

            transform_vertex(&p, size, center);

            {
                fx2_t v;
                v.x = p.x;
                v.y = p.z;
                v = fx_rotate_xy(v, c, s);

                p.x = v.x;
                p.z = v.y;
            }

            p.z += 1400;

            {
                fx2_t v;
                v = project_to_screen(p.x, p.y, p.z);
                tm_buffer[i++] = fx_clamp(v.x, INT16_MIN, INT16_MAX);
                tm_buffer[i++] = fx_clamp(v.y, INT16_MIN, INT16_MAX);
            }
        }
    }

    {
        int16_t __far* draw_buffer_tgt = draw_buffer + draw_buffer_position;
        const uint8_t* indices_end = indices + num_indices;
        while (indices < indices_end) {
            uint16_t a, b, c;
            fx_t x0, y0, x1, y1, x2, y2;

            a = *indices++;
            b = *indices++;
            c = *indices++;

            a *= 2;
            b *= 2;
            c *= 2;

            x0 = tm_buffer[a + 0];
            y0 = tm_buffer[a + 1];
            x2 = tm_buffer[b + 0];
            y2 = tm_buffer[b + 1];
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
            }
        }

        draw_buffer_position += draw_buffer_tgt - draw_buffer;
    }
}

static void flush_draw_buffer() {
    int16_t x0, y0, x1, y1, x2, y2;
    uint8_t c;
    uint16_t i;
    int16_t __far* draw_buffer_src = draw_buffer;

    for (i = 0; i < draw_buffer_position; i += 8) {
        x0 = *draw_buffer_src++;
        y0 = *draw_buffer_src++;
        x1 = *draw_buffer_src++;
        y1 = *draw_buffer_src++;
        x2 = *draw_buffer_src++;
        y2 = *draw_buffer_src++;
        c = *draw_buffer_src++;
        draw_buffer_src++; // Reserved

        draw_tri(x0, y0, x1, y1, x2, y2, 6);
        draw_triangle_lines(x0, y0, x1, y1, x2, y2, 7);
     }

     draw_buffer_position = 0;
}

#endif
