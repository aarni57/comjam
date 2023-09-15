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

    draw_tri(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, 122);

    draw_line(v0.x >> RASTER_SUBPIXEL_BITS, v0.y >> RASTER_SUBPIXEL_BITS, v1.x >> RASTER_SUBPIXEL_BITS, v1.y >> RASTER_SUBPIXEL_BITS, 123);
    draw_line(v1.x >> RASTER_SUBPIXEL_BITS, v1.y >> RASTER_SUBPIXEL_BITS, v2.x >> RASTER_SUBPIXEL_BITS, v2.y >> RASTER_SUBPIXEL_BITS, 123);
    draw_line(v2.x >> RASTER_SUBPIXEL_BITS, v2.y >> RASTER_SUBPIXEL_BITS, v0.x >> RASTER_SUBPIXEL_BITS, v0.y >> RASTER_SUBPIXEL_BITS, 123);
}

static void draw_fps() {
    char buf[3] = { 0 };
    uint8_t tens = fps.average / 10;
    buf[0] = tens ? '0' + tens : ' ';
    buf[1] = '0' + (fps.average - tens * 10);
    draw_text(buf, 320 - 12, 2, 4);
}

static void draw() {
    draw_test_triangle();

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
#if 0
    vga_set_palette(253, 0, 20, 0);
    vga_set_palette(254, 0, 48, 0);
    vga_set_palette(255, 0, 58, 0);
#endif

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
