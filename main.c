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

static void draw_test_triangle() {
    static uint8_t c = 0;
    static uint32_t x = 0;
    int32_t x0, y0, x1, y1, x2, y2;

    c++;
    x += frame_dt / 1000;
    x &= 255;

    x0 = x;
    y0 = 40;
    x1 = x + 40;
    y1 = 160;
    x2 = x + 240;
    y2 = 180;

    x0 <<= RASTER_SUBPIXEL_BITS;
    y0 <<= RASTER_SUBPIXEL_BITS;
    x1 <<= RASTER_SUBPIXEL_BITS;
    y1 <<= RASTER_SUBPIXEL_BITS;
    x2 <<= RASTER_SUBPIXEL_BITS;
    y2 <<= RASTER_SUBPIXEL_BITS;

    tri(x0, y0, x1, y1, x2, y2, c, dblbuf);
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
