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
#include "ship.h"
#include "asteroid.h"

//

static uint8_t __far* dblbuf = NULL;

#include "drawtext.h"
#include "draw.h"

//

#if 0
static int opl = 0;
#include "opl.h"
#endif

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
static int help = 0;
static int draw_mode = 0;
static int stars_enabled = 1;
static int asteroids_enabled = 1;
static int ship_enabled = 1;
static int texts_enabled = 1;
static int fps_enabled = 1;

static struct {
    uint32_t time_accumulator;
    uint32_t frame_accumulator;
    uint32_t average;
} fps = { 0 };

#define TICK_LENGTH_US (1000000UL / 60)

static struct {
    uint32_t current_frame;
    uint32_t tick_accumulator;
    uint32_t current_tick;
    uint16_t ticks_to_advance;
} timing = { 0 };

//

static void update_timing(uint32_t dt_us) {
    timing.current_frame++;
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

#if 0
            case 'a':
                opl_play();
                break;
#endif

            case '1':
                stars_enabled ^= 1;
                break;

            case '2':
                asteroids_enabled ^= 1;
                break;

            case '3':
                ship_enabled ^= 1;
                break;

            case '4':
                texts_enabled ^= 1;
                break;

            case '5':
                fps_enabled ^= 1;
                break;

            case 'v':
                vsync ^= 1;
                break;

            case 'w':
                draw_mode ^= 1;
                break;

            default:
                help ^= 1;
                break;
        }
    }
}

static inline uint32_t xorshift32() {
    static uint32_t x = 0xcafebabe;
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static inline fx_random_one() {
    return xorshift32() & (FX_ONE - 1);
}

static inline fx_random_signed_one() {
    return (xorshift32() & (FX_ONE * 2 - 1)) - FX_ONE;
}

#define NUM_ASTEROIDS 16
static fx3_t asteroid_positions[NUM_ASTEROIDS];
static fx_t asteroid_speeds[NUM_ASTEROIDS];
static fx3_t asteroid_rotation_axes[NUM_ASTEROIDS];

static void init_asteroids() {
    uint16_t i;
    for (i = 0; i < NUM_ASTEROIDS; ++i) {
        fx3_t r;
        r.x = fx_random_signed_one() >> 5;
        r.y = fx_random_signed_one() >> 5;
        r.z = fx_random_signed_one() >> 5;
        asteroid_positions[i] = r;

        asteroid_speeds[i] = (fx_random_one() >> 12) + (FX_ONE >> 12);

        r.x = fx_random_signed_one();
        r.y = fx_random_signed_one();
        r.z = fx_random_signed_one();
        fx3_normalize_ip(&r);
        asteroid_rotation_axes[i] = r;
    }
}

static void update_asteroids() {
    uint16_t i;
    for (i = 0; i < NUM_ASTEROIDS; ++i) {
        asteroid_positions[i].y -= asteroid_speeds[i];
        if (asteroid_positions[i].y <= -2048) {
            asteroid_positions[i].y += 4096;
            asteroid_positions[i].x = fx_random_signed_one() >> 5;
            asteroid_positions[i].z = fx_random_signed_one() >> 5;
            asteroid_speeds[i] = (fx_random_one() >> 12) + (FX_ONE >> 12);
        }
    }
}

static void update() {
    uint16_t i = 0;
    for (i = 0; i < timing.ticks_to_advance; ++i) {
        update_asteroids();
    }
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

const fx3_t AXIS_X = { FX_ONE, 0, 0 };
const fx3_t AXIS_Y = { 0, FX_ONE, 0 };
const fx3_t AXIS_Z = { 0, 0, FX_ONE };

static void setup_view_matrix(fx4x3_t* view_matrix) {
    fx_t rotation_angle_x = fx_sin((fx_t)timing.current_tick * 27) / 10;
    fx_t rotation_angle_z = FX_HALF + (fx_t)timing.current_tick * 40;
    fx4_t view_rotation_x;
    fx4_t view_rotation_z;
    fx4_t view_rotation;
    fx3_t view_translation = { 0, 0, 1400 };

    fx_quat_rotation_axis_angle(&view_rotation_x, &AXIS_X, -FX_ONE / 4 + rotation_angle_x);
    fx_quat_rotation_axis_angle(&view_rotation_z, &AXIS_Z, rotation_angle_z);
    fx_quat_mul(&view_rotation, &view_rotation_x, &view_rotation_z);

    fx4x3_rotation_translation(view_matrix, &view_rotation, &view_translation);
}

#define NUM_STARS 128
static fx3_t star_positions[NUM_STARS];

static void init_stars() {
    uint16_t i;
    for (i = 0; i < NUM_STARS; ++i) {
        fx3_t v;
        v.x = fx_random_signed_one();
        v.y = fx_random_signed_one();
        v.z = fx_random_signed_one();
        fx3_normalize_ip(&v);
        star_positions[i] = v;
    }
}

static void draw_stars(const fx4x3_t* view_matrix) {
    int16_t x, y;
    uint16_t i, offset;
    fx3_t v;

    for (i = 0; i < NUM_STARS; ++i) {
        fx_transform_vector(&v, view_matrix, &star_positions[i]);

        if (v.z < NEAR_CLIP)
            continue;

        project_to_screen(&v);

        v.x >>= RASTER_SUBPIXEL_BITS;
        v.y >>= RASTER_SUBPIXEL_BITS;

        switch (i & 7) {
            case 0:
            case 1: {
                if (v.x >= 0 && v.x < SCREEN_WIDTH && v.y >= 0 && v.y < SCREEN_HEIGHT) {
                    offset = mul_by_screen_stride(v.y) + v.x;
                    dblbuf[offset] = 96;
                }

                break;
            }

            case 2:
            case 3:
            case 4: {
                if (v.x >= 0 && v.x < SCREEN_WIDTH && v.y >= 0 && v.y < SCREEN_HEIGHT) {
                    offset = mul_by_screen_stride(v.y) + v.x;
                    dblbuf[offset] = 9;
                }

                break;
            }

            case 5:
            case 6: {
                if (v.x >= 1 && v.x < SCREEN_WIDTH - 1 && v.y >= 1 && v.y < SCREEN_HEIGHT - 1) {
                    offset = mul_by_screen_stride(v.y) + v.x;
                    dblbuf[offset - SCREEN_WIDTH] = 8;
                    dblbuf[offset - 1] = 8;
                    dblbuf[offset] = 117;
                    dblbuf[offset + 1] = 8;
                    dblbuf[offset + SCREEN_WIDTH] = 8;
                }

                break;
            }

            case 7: {
                if (v.x >= 1 && v.x < SCREEN_WIDTH - 1 && v.y >= 1 && v.y < SCREEN_HEIGHT - 1) {
                    offset = mul_by_screen_stride(v.y) + v.x;
                    dblbuf[offset - SCREEN_WIDTH] = 9;
                    dblbuf[offset - 1] = 9;
                    dblbuf[offset] = 105;
                    dblbuf[offset + 1] = 9;
                    dblbuf[offset + SCREEN_WIDTH] = 9;
                }

                break;
            }
        }
    }
}

static fx4x3_t ship_mesh_adjust_matrix;
static fx4x3_t asteroid_mesh_adjust_matrix;

static void draw_ship(const fx4x3_t* view_matrix) {
    fx4x3_t model_matrix, tmp, model_view_matrix;
    fx3_t model_translation;
    fx_t t1, t2;

    t1 = (fx_t)timing.current_tick * 22;
    t2 = (fx_t)timing.current_tick * 27;
    model_translation.x = fx_cos(t1) >> 7;
    model_translation.y = 0;
    model_translation.z = (fx_sin(t2) >> 8) + 64;

    fx4x3_translation(&model_matrix, &model_translation);

    fx4x3_mul(&tmp, &model_matrix, &ship_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        ship_num_indices, ship_num_vertices,
        ship_indices, ship_face_colors, ship_vertices);
}

static void draw_asteroid(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &asteroid_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        asteroid_num_indices, asteroid_num_vertices,
        asteroid_indices, asteroid_face_colors, asteroid_vertices);
}

static void draw_asteroids(const fx4x3_t* view_matrix) {
    fx4_t rotation;
    fx_t rotation_angle;
    uint16_t i;
    for (i = 0; i < NUM_ASTEROIDS; ++i) {
        rotation_angle = asteroid_positions[i].y * 80;
        fx_quat_rotation_axis_angle(&rotation, &asteroid_rotation_axes[i], rotation_angle);
        draw_asteroid(view_matrix, &rotation, &asteroid_positions[i]);
    }
}

static const char* const TEXT = "2669, Gemini sector, Troy system";

static void draw_texts() {
    char buffer[64];
    int16_t x;
    uint8_t i;
    uint32_t t = timing.current_tick >> 3;

    for (i = 0; i < 64; ++i) {
        uint32_t t_step;

        if (TEXT[i] == 0)
            break;

        if (i == 0) {
            t_step = 10;
        } else {
            t_step = TEXT[i] == ' ' ? 2 : 1;
        }

        if (t < t_step)
            break;

        t -= t_step;

        buffer[i] = TEXT[i];
    }

    buffer[i] = 0;

    x = 4;
    x = draw_text(buffer, x, 200 - (6 + 4), 4);
    draw_text_cursor(x, 200 - (6 + 4), 7);
}

static void draw() {
    fx4x3_t view_matrix;
    setup_view_matrix(&view_matrix);

    if (stars_enabled)
        draw_stars(&view_matrix);

    if (ship_enabled)
        draw_ship(&view_matrix);

    if (asteroids_enabled)
        draw_asteroids(&view_matrix);

    flush_mesh_draw_buffer(draw_mode);

    if (texts_enabled) {
        if (help) {
            int16_t y = 4;
            draw_text("build 2023-09-25", 4, y, 4); y += 12;
            draw_text("Sorry, this is nonplayable.", 4, y, 4); y += 12;
            draw_text("v: Toggle vertical sync", 4, y, 4); y += 6;
            draw_text("w: Change draw mode (solid/wireframe)", 4, y, 4); y += 6;
            draw_text("1-5: Toggle rendering (stars, asteroids, ship, texts, fps)", 4, y, 4); y += 6;
            draw_text("esc: Exit to DOS", 4, y, 4); y += 12;
            draw_text("Made for DOS COM Jam 2023", 4, y, 4); y += 6;
            draw_text("https://aarnig.itch.io/dos-com-jam", 4, y, 4); y += 6;
            draw_text("aarni.gratseff@gmail.com", 4, y, 4); y += 6;
        } else {
            draw_texts();
        }
    }

    if (fps_enabled)
        draw_fps();
}

void main() {
    aw_assert(ship_num_indices % 3 == 0);
    aw_assert(sizeof(ship_vertices) == ship_num_vertices * 3);
    aw_assert(sizeof(ship_indices) == ship_num_indices * sizeof(uint16_t));
    aw_assert(sizeof(ship_face_colors) == ship_num_indices / 3);

    //

    fx4x3_identity(&ship_mesh_adjust_matrix);
    ship_mesh_adjust_matrix.m[FX4X3_00] = ship_size.x << 1;
    ship_mesh_adjust_matrix.m[FX4X3_11] = ship_size.y << 1;
    ship_mesh_adjust_matrix.m[FX4X3_22] = ship_size.z << 1;
    ship_mesh_adjust_matrix.m[FX4X3_30] = ship_center.x >> 7;
    ship_mesh_adjust_matrix.m[FX4X3_31] = ship_center.y >> 7;
    ship_mesh_adjust_matrix.m[FX4X3_32] = ship_center.z >> 7;

    fx4x3_identity(&asteroid_mesh_adjust_matrix);
    asteroid_mesh_adjust_matrix.m[FX4X3_00] = asteroid_size.x << 1;
    asteroid_mesh_adjust_matrix.m[FX4X3_11] = asteroid_size.y << 1;
    asteroid_mesh_adjust_matrix.m[FX4X3_22] = asteroid_size.z << 1;
    asteroid_mesh_adjust_matrix.m[FX4X3_30] = asteroid_center.x >> 7;
    asteroid_mesh_adjust_matrix.m[FX4X3_31] = asteroid_center.y >> 7;
    asteroid_mesh_adjust_matrix.m[FX4X3_32] = asteroid_center.z >> 7;

    init_stars();
    init_asteroids();

    //

    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf) {
        goto exit;
    }

    tm_buffer = (int16_t __far*)_fmalloc(sizeof(int16_t) * TM_BUFFER_SIZE);
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

#if 0
    opl_init();
#endif

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
#if 0
    opl_done();
#endif
    vga_set_mode(0x3);
    putz(exit_message);
    set_text_cursor(1, 0);
    kb_clear_buffer();
}
