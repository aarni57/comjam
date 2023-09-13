#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#include "vga.h"
#include "minmax.h"
#include "tri.h"
#include "util.h"

//

static uint8_t __far* dblbuf = NULL;

#include "drawtext.h"

//

static int opl_enabled = 0;
static uint16_t opl_base = 0x388;

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

static uint32_t fps_time_accumulator = 0;
static uint32_t fps_frame_accumulator = 0;
static uint32_t fps = 0;

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

static void update() {

}

#if 1
#define abs16 abs
#else
static inline int16_t abs16(int16_t x) {
    return x < 0 ? -x : x;
}
#endif

#define TRI_SPLITTING_THRESHOLD 0x8000

static void draw_tri(
    fx_t x0, fx_t y0,
    fx_t x1, fx_t y1,
    fx_t x2, fx_t y2,
    uint8_t c) {
    if ((fx_abs(x0 - x1) | fx_abs(y0 - y1)) & TRI_SPLITTING_THRESHOLD) {
        fx_t sx = (x0 + x1) / 2;
        fx_t sy = (y0 + y1) / 2;
        draw_tri(x2, y2, x0, y0, sx, sy, x2);
        draw_tri(x1, y1, x2, y2, sx, sy, c);
        return;
    }

    if ((fx_abs(x1 - x2) | fx_abs(y1 - y2)) & TRI_SPLITTING_THRESHOLD) {
        fx_t sx = (x1 + x2) / 2;
        fx_t sy = (y1 + y2) / 2;
        draw_tri(x0, y0, x1, y1, sx, sy, x0);
        draw_tri(x2, y2, x0, y0, sx, sy, c);
        return;
    }

    if ((fx_abs(x2 - x0) | fx_abs(y2 - y0)) & TRI_SPLITTING_THRESHOLD) {
        fx_t sx = (x0 + x2) / 2;
        fx_t sy = (y0 + y2) / 2;
        draw_tri(x1, y1, x2, y2, sx, sy, x1);
        draw_tri(x0, y0, x1, y1, sx, sy, c);
        return;
    }

    tri(x0, y0, x1, y1, x2, y2, c, dblbuf);
}

static inline fx2_t transform_to_screen(fx2_t v) {
    fx2_t r;
    r.x = v.x;
    r.y = -v.y * 5 / 6;
    r.x += RASTER_SCREEN_CENTER_X;
    r.y += RASTER_SCREEN_CENTER_Y;
    return r;
}

static void draw_test_triangle() {
    static fx_t t = 0;
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
}

static void draw_fps() {
    char buf[3] = { 0 };
    uint8_t tens = fps / 10;
    buf[0] = tens ? '0' + tens : ' ';
    buf[1] = '0' + (fps - tens * 10);
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

        fps_time_accumulator += frame_dt;
        if (fps_time_accumulator >= 1000000) {
            fps = (fps_frame_accumulator * 1000000) / fps_time_accumulator;
            fps_time_accumulator = 0;
            fps_frame_accumulator = 0;
        }

        fps_frame_accumulator++;

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
