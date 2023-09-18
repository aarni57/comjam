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

#include "drawtext.h"
#include "draw.h"

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

static void draw_fps() {
    char buf[3] = { 0 };
    uint8_t tens = fps.average / 10;
    buf[0] = tens ? '0' + tens : ' ';
    buf[1] = '0' + (fps.average - tens * 10);
    draw_text(buf, 320 - 12, 2, 4);
}

static void draw() {
    draw_mesh(torus_num_indices, torus_num_vertices,
        &torus_center, &torus_size, t / 128, torus_indices, torus_vertices);

    flush_draw_buffer();

    draw_text("Prerendering graphics...", 6, 6, 4);
    draw_text_cursor(6, 12, 7);

    draw_fps();
}

void main() {
    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        goto exit;
    }

    draw_buffer = (int16_t __far*)_fmalloc(sizeof(int16_t) * DRAW_BUFFER_SIZE);
    if (!draw_buffer) {
        goto exit;
    }

    sort_buffer = (uint32_t __far*)_fmalloc(sizeof(uint32_t) * DRAW_BUFFER_SIZE);
    if (!sort_buffer) {
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

exit:
    _ffree(dblbuf);
    _ffree(draw_buffer);
    _ffree(sort_buffer);
    timer_cleanup();
    opl_done();
    vga_set_mode(0x3);
    putz(exit_message);
    set_text_cursor(2, 0);
    kb_clear_buffer();
}
