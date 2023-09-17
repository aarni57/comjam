#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

//

#if 0
#   include <assert.h>
#   define AW_ASSERT assert
#else
#   define AW_ASSERT(x)
#endif

#define abs16 abs
#define abs32 abs

#define swap16(a, b) { int16_t t = a; a = b; b = t; }
#define swap32(a, b) { int32_t t = a; a = b; b = t; }

//

#include "vga.h"
#include "minmax.h"
#include "util.h"
#include "pal.h"
#include "torus.h"

//

static uint8_t __far* dblbuf = NULL;

#include "tri.h"
#include "line.h"
#include "drawtext.h"

//

#define OPL_BASE_DEFAULT 0x388

static int opl = 0;
static uint16_t opl_base = OPL_BASE_DEFAULT;

#include "opl.h"

//

volatile uint32_t timer_ticks;

// defined in timer.asm
void timer_init();
void timer_cleanup();

#define TIMER_TICK_USEC 858

//

static const char* exit_message = "JEMM unloaded... Not really :-)";

static int quit = 0;

static uint32_t frame_dt = 0;

static struct {
    uint32_t time_accumulator;
    uint32_t frame_accumulator;
    uint32_t average;
} fps = { 0 };

//

static void update_input() {
    if (kbhit()) {
        char ch = getch();
        switch (ch) {
            case 27:
                quit = 1;
                break;

            case 'a':
                opl_play();
                break;

            default:
                break;
        }
    }
}

static fx_t t = 0;

static void update() {
    t += frame_dt;
}

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

static void draw_mesh(uint16_t num_indices, uint16_t num_vertices,
    fx_t center_x, fx_t center_y, fx_t center_z,
    fx_t size_x, fx_t size_y, fx_t size_z,
    const uint8_t* indices, const int8_t* vertices) {
    static fx_t tm_buffer[256 * 2];

    {
        uint16_t i = 0, j;
        uint16_t j_end = num_vertices * 3;
        fx_t c, s;
        fx_t t2;

        t2 = t / 128;
        c = fx_cos(t2);
        s = fx_sin(t2);

        for (j = 0; j < j_end; j += 3) {
            fx_t x, y, z;

            x = vertices[j + 0];
            y = vertices[j + 1];
            z = vertices[j + 2];

            x *= size_x;
            y *= size_y;
            z *= size_z;

            x += center_x;
            y += center_y;
            z += center_z;

            x >>= 7;
            y >>= 7;
            z >>= 7;

            {
                fx2_t v;
                v.x = x;
                v.y = z;
                v = fx_rotate_xy(v, c, s);

                x = v.x;
                z = v.y;
            }

            z += 1400;

            {
                fx2_t v;
                v = project_to_screen(x, y, z);
                tm_buffer[i++] = v.x;
                tm_buffer[i++] = v.y;
            }
        }
    }

    {
        uint16_t i;
        for (i = 0; i < num_indices; i += 3) {
            uint16_t a, b, c;
            fx_t x0, y0, x1, y1, x2, y2;

            a = indices[i + 0];
            b = indices[i + 2];
            c = indices[i + 1];

            a *= 2;
            b *= 2;
            c *= 2;

            x0 = tm_buffer[a + 0];
            y0 = tm_buffer[a + 1];
            x1 = tm_buffer[b + 0];
            y1 = tm_buffer[b + 1];
            x2 = tm_buffer[c + 0];
            y2 = tm_buffer[c + 1];

            if (calc_triangle_area(x0, y0, x1, y1, x2, y2) > 0) {
                draw_tri(x0, y0, x1, y1, x2, y2, 6);
                draw_triangle_lines(x0, y0, x1, y1, x2, y2, 7);
            }
        }
    }
}

static void draw_fps() {
    char buf[3] = { 0 };
    uint8_t tens = fps.average / 10;
    buf[0] = tens ? '0' + tens : ' ';
    buf[1] = '0' + (fps.average - tens * 10);
    draw_text(buf, 320 - 12, 2, 4);
}

static void draw() {
    //draw_test_triangle();
    draw_mesh(torus_num_indices, torus_num_vertices,
        torus_center_x, torus_center_y, torus_center_z,
        torus_size_x, torus_size_y, torus_size_z,
        torus_indices, torus_vertices);

    draw_text("Prerendering graphics...", 6, 6, 4);
    draw_text_cursor(6, 12, 7);

    draw_fps();
}

void main() {
    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        goto exit;
    }

    opl_init();
    timer_init();

    vga_set_mode(0x13);

    {
        uint16_t i;
        const uint8_t* src = PALETTE;
        for (i = 0; i < NUM_PALETTE_COLORS; ++i) {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;
            r >>= 2;
            g >>= 2;
            b >>= 2;
            vga_set_palette(i, r, g, b);
        }
    }

    while (!quit) {
        static uint32_t previous_frame_ticks = 0;
        uint32_t frame_start_ticks, ticks_delta;

        _disable();
        frame_start_ticks = timer_ticks;
        _enable();

        ticks_delta = frame_start_ticks - previous_frame_ticks;
        previous_frame_ticks = frame_start_ticks;
        frame_dt = ticks_delta * TIMER_TICK_USEC;

        fps.time_accumulator += frame_dt;
        if (fps.time_accumulator >= 1000000) {
            fps.average = (fps.frame_accumulator * 1000000) / fps.time_accumulator;
            fps.time_accumulator = 0;
            fps.frame_accumulator = 0;
        }

        fps.frame_accumulator++;

        update_input();
        update();

        _fmemset(dblbuf, 0, SCREEN_NUM_PIXELS);
        draw();
        vga_wait_for_retrace();
        _fmemcpy(VGA, dblbuf, SCREEN_NUM_PIXELS);
    }

    _ffree(dblbuf);

exit:
    timer_cleanup();
    opl_done();
    vga_set_mode(0x3);
    putz(exit_message);
    set_text_cursor(2, 0);
    kb_clear_buffer();
}
