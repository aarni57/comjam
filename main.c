#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#include "vga.h"
#include "minmax.h"
#include "util.h"

//

static uint8_t __far* dblbuf = NULL;

#include "tri.h"
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

}

#define abs32 abs
#define swap32(a, b) { int32_t t = a; a = b; b = t; }

static inline void put_pixel(int16_t x, int16_t y, uint8_t c) {
    dblbuf[x + mul_by_screen_stride(y)] = c;
}

static inline void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c) {
    uint8_t steep = 0;
    int16_t dx, dy, error2, derror2, x, y;

    if (abs32(x0 - x1) < abs32(y0 - y1)) {
        swap32(x0, y0);
        swap32(x1, y1);
        steep = 1;
    }

    if (x0 > x1) {
        swap32(x0, x1);
        swap32(y0, y1);
    }

    dx = x1 - x0;
    dy = y1 - y0;
    derror2 = abs32(dy) << 1;
    error2 = 0;
    y = y0;

    if (steep) {
        if (y1 > y0) {
            if (y >= SCREEN_WIDTH)
                return;

            for (x = x0; x <= x1; x++) {
                if (x >= 0 && y >= 0 && x < SCREEN_HEIGHT)
                    put_pixel(y, x, c);

                error2 += derror2;
                if (error2 > dx) {
                    y++;
                    if (y == SCREEN_WIDTH)
                        return;
                    error2 -= dx << 1;
                }
            }
        } else {
            if (y < 0)
                return;

            for (x = x0; x <= x1; x++) {
                if (x >= 0 && y < SCREEN_WIDTH && x < SCREEN_HEIGHT)
                    put_pixel(y, x, c);

                error2 += derror2;
                if (error2 > dx) {
                    if (y == 0)
                        return;
                    y--;
                    error2 -= dx << 1;
                }
            }
        }
    } else {
        if (y1 > y0) {
            if (y >= SCREEN_HEIGHT)
                return;

            for (x = x0; x <= x1; x++) {
                if (x >= 0 && y >= 0 && x < SCREEN_WIDTH)
                    put_pixel(x, y, c);

                error2 += derror2;
                if (error2 > dx) {
                    y++;
                    if (y == SCREEN_HEIGHT)
                        return;
                    error2 -= dx << 1;
                }
            }
        } else {
            if (y < 0)
                return;

            for (x = x0; x <= x1; x++) {
                if (x >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
                    put_pixel(x, y, c);

                error2 += derror2;
                if (error2 > dx) {
                    if (y == 0)
                        return;
                    y--;
                    error2 -= dx << 1;
                }
            }
        }
    }
}

static void draw_test_triangle() {
    fx_t t2, t3, scale, c, s;
    fx2_t v0, v1, v2;

    v0.x = 0;
    v0.y = -1500;
    v1.x = 1100;
    v1.y = 1000;
    v2.x = -1100;
    v2.y = 1000;

    t += frame_dt;
    t2 = t / 128;
    t3 = t / 96;

    scale = (fx_sin(t3) >> 8) + 300;
    c = (fx_cos(t2) * scale) >> 8;
    s = (fx_sin(t2) * scale) >> 8;

    v0 = fx_rotate_xy(v0, c, s);
    v1 = fx_rotate_xy(v1, c, s);
    v2 = fx_rotate_xy(v2, c, s);

    v0 = transform_to_screen(v0);
    v1 = transform_to_screen(v1);
    v2 = transform_to_screen(v2);

    draw_tri(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, 1);

    draw_line(v0.x >> RASTER_SUBPIXEL_BITS, v0.y >> RASTER_SUBPIXEL_BITS, v1.x >> RASTER_SUBPIXEL_BITS, v1.y >> RASTER_SUBPIXEL_BITS, 10);
    draw_line(v1.x >> RASTER_SUBPIXEL_BITS, v1.y >> RASTER_SUBPIXEL_BITS, v2.x >> RASTER_SUBPIXEL_BITS, v2.y >> RASTER_SUBPIXEL_BITS, 10);
    draw_line(v2.x >> RASTER_SUBPIXEL_BITS, v2.y >> RASTER_SUBPIXEL_BITS, v0.x >> RASTER_SUBPIXEL_BITS, v0.y >> RASTER_SUBPIXEL_BITS, 10);
}

static void draw_fps() {
    char buf[3] = { 0 };
    uint8_t tens = fps.average / 10;
    buf[0] = tens ? '0' + tens : ' ';
    buf[1] = '0' + (fps.average - tens * 10);
    draw_text(buf, 320 - 12, 2, 252);
}

static void draw() {
    draw_test_triangle();

    draw_text("Prerendering graphics...", 6, 6, 252);
    draw_text_cursor(6, 12, 255);

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
    vga_set_palette(253, 0, 20, 0);
    vga_set_palette(254, 0, 48, 0);
    vga_set_palette(255, 0, 58, 0);

#if 0
    {
        uint8_t i, r, g, b;
        for (i = 0; i < 64; ++i) {
            r = minu8(i * 3 / 2, 63);
            g = i > 32 ? minu8((i - 32) << 1, 63) : 0;
            b = i > 52 ? minu8((i - 52) << 2, 63) : 0;
            vga_set_palette(i, r, g, b);
        }
    }
#endif

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
