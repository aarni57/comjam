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
#   define aw_assert assert
#else
#   define aw_assert(x)
#endif

#define abs16 abs

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

static int opl = 0;

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
static int vsync = 0;

static struct {
    uint32_t time_accumulator;
    uint32_t frame_accumulator;
    uint32_t average;
} fps = { 0 };

#define TICK_LENGTH_US (1000000UL / 60)

static struct {
    uint32_t tick_accumulator;
    uint32_t current_tick;
    uint32_t ticks_to_advance;
} timing = { 0 };

//

static void update_timing(uint32_t dt_us) {
    timing.ticks_to_advance = 0;
    timing.tick_accumulator += dt_us;
    while (timing.tick_accumulator >= TICK_LENGTH_US) {
        timing.tick_accumulator -= TICK_LENGTH_US;
        timing.current_tick++;
        timing.ticks_to_advance++;
    }
}

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

            case 'v':
                vsync ^= 1;
                break;

            default:
                break;
        }
    }
}

static void update() {
}

static void draw_fps() {
    char buf[5] = { 0 };
    uint8_t ones, tens, hundreds;
    uint16_t v = minu32(fps.average, 999);

#if 0
    hundreds = v / 100;
    v -= hundreds * 100;
    tens = v / 10;
    v -= tens * 10;
    ones = v;
#else
    __asm {
        .386
        mov ax, v
        mov cx, 100
        xor dx, dx
        div cx
        mov hundreds, al
        mov ax, dx
        mov cx, 10
        xor dx, dx
        div cx
        mov tens, al
        mov ones, dl
    }
#endif

    buf[0] = vsync ? '*' : ' ';
    buf[1] = hundreds ? '0' + hundreds : ' ';
    buf[2] = (hundreds || tens) ? '0' + tens : ' ';
    buf[3] = '0' + ones;
    draw_text(buf, 320 - 24, 4, 4);
}

static void draw() {
    fx4x3_t view_matrix, model_matrix, mesh_adjust_matrix, tmp, model_view_matrix;
    fx4_t model_rotation;
    fx3_t model_rotation_axis = { 0, FX_ONE, 0 };
    fx3_t model_translation = { 0 };
    fx_t model_rotation_angle = timing.current_tick * 64;

    model_translation.x = fx_sin(timing.current_tick * 32) >> 6;

    fx4x3_identity(&view_matrix);
    view_matrix.m[FX4X3_32] = 2048;

    fx_quat_rotation_axis_angle(&model_rotation, &model_rotation_axis, model_rotation_angle);

    fx4x3_rotation_translation(&model_matrix, &model_rotation, &model_translation);

    fx4x3_identity(&mesh_adjust_matrix);
    mesh_adjust_matrix.m[FX4X3_00] = torus_size.x << 9;
    mesh_adjust_matrix.m[FX4X3_11] = torus_size.y << 9;
    mesh_adjust_matrix.m[FX4X3_22] = torus_size.z << 9;
    mesh_adjust_matrix.m[FX4X3_30] = torus_center.x;
    mesh_adjust_matrix.m[FX4X3_31] = torus_center.y;
    mesh_adjust_matrix.m[FX4X3_32] = torus_center.z;

    fx4x3_mul(&tmp, &model_matrix, &mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, &view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        torus_num_indices, torus_num_vertices,
        torus_indices, torus_face_colors, torus_vertices);

    flush_mesh_draw_buffer();

    draw_text("Prerendering graphics...", 4, 4, 4);
    draw_text_cursor(4, 10, 7);

    draw_fps();
}

void main() {
    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        goto exit;
    }

    tm_buffer = (fx_t __far*)_fmalloc(sizeof(fx_t) * TM_BUFFER_SIZE);
    if (!tm_buffer) {
        goto exit;
    }

    draw_buffer = (int16_t __far*)_fmalloc(sizeof(int16_t) * 8 * DRAW_BUFFER_MAX_TRIANGLES);
    if (!draw_buffer) {
        goto exit;
    }

    sort_buffer = (uint32_t __far*)_fmalloc(sizeof(uint32_t) * DRAW_BUFFER_MAX_TRIANGLES);
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
            vga_set_palette(i, r >> 2, g >> 2, b >> 2);
        }
    }

    while (!quit) {
        static uint32_t previous_frame_ticks = 0;
        uint32_t frame_start_ticks, ticks_delta, frame_dt;

        _disable();
        frame_start_ticks = timer_ticks;
        _enable();

        ticks_delta = frame_start_ticks - previous_frame_ticks;
        previous_frame_ticks = frame_start_ticks;
        frame_dt = mul32(ticks_delta, TIMER_TICK_USEC);

        fps.time_accumulator += frame_dt;
        if (fps.time_accumulator >= 1000000) {
            fps.average = div32(mul32(fps.frame_accumulator, 1000000), fps.time_accumulator);
            fps.time_accumulator = 0;
            fps.frame_accumulator = 0;
        }

        fps.frame_accumulator++;

        update_timing(frame_dt);
        update_input();
        update();

#if 0
        _fmemset(dblbuf, 0, SCREEN_NUM_PIXELS);
#else
        __asm {
            push es
            push edi

            xor eax, eax

            mov edx, dblbuf
            movzx edi, dx
            shr edx, 16
            mov es, dx

            mov cx, SCREEN_NUM_PIXELS
            shr cx, 2

            rep stosd

            pop edi
            pop es
        }
#endif

        draw();

        if (vsync)
            vga_wait_for_retrace();

#if 0
        _fmemcpy(VGA, dblbuf, SCREEN_NUM_PIXELS);
#else
        __asm {
            push es
            push ds
            push edi
            push esi

            mov edx, dblbuf
            movzx esi, dx
            shr edx, 16
            mov ds, dx

            mov dx, 0xa000
            mov es, dx
            xor edi, edi

            mov cx, SCREEN_NUM_PIXELS
            shr cx, 2

            rep movsd

            pop esi
            pop edi
            pop ds
            pop es
        }
#endif
    }

exit:
    _ffree(dblbuf);
    _ffree(tm_buffer);
    _ffree(draw_buffer);
    _ffree(sort_buffer);
    timer_cleanup();
    opl_done();
    vga_set_mode(0x3);
    putz(exit_message);
    set_text_cursor(1, 0);
    kb_clear_buffer();
}
