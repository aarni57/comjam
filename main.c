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
#include "scrap.h"

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

static inline uint32_t read_timer_ticks() {
    uint32_t r;
    _disable();
    r = timer_ticks;
    _enable();
    return r;
}

#define TIMER_TICK_USEC 858

//

static const char* exit_message = "JEMM unloaded... Not really :-)";

static int quit = 0;
static int vsync = 0;
static int help = 0;
static int draw_mode = 0;
static int stars_enabled = 1;
static int debris_enabled = 1;
static int ship_enabled = 1;
static int texts_enabled = 1;
static int fps_enabled = 1;

static int benchmark_timer = 0;
static uint32_t benchmark_start_tick = 0;
static uint32_t benchmark_result = 0;

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

#if 0
#define XORSHIFT32_INITIAL_VALUE 0xcafebabe
static uint32_t xorshift32_state = XORSHIFT32_INITIAL_VALUE;

static inline uint32_t xorshift32() {
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    xorshift32_state ^= xorshift32_state << 13;
    xorshift32_state ^= xorshift32_state >> 17;
    xorshift32_state ^= xorshift32_state << 5;
    return xorshift32_state;
}

static inline void xorshift32_reset() {
    xorshift32_state = XORSHIFT32_INITIAL_VALUE;
}
#else
uint32_t xorshift32();
void xorshift32_reset();
#endif

#if 0
static inline fx_t fx_random_one() {
    return xorshift32() & (FX_ONE - 1);
}

static inline fx_t fx_random_signed_one() {
    return (xorshift32() & (FX_ONE * 2 - 1)) - FX_ONE;
}

static inline fx_t random_debris_x() {
    return fx_random_signed_one() >> 5;
}
#else
fx_t fx_random_one();
fx_t fx_random_signed_one();
fx_t random_debris_x();
#endif

#define NUM_DEBRIS 16
static fx3_t debris_positions[NUM_DEBRIS];
static fx3_t debris_rotation_axes[NUM_DEBRIS];
static fx_t debris_speeds[NUM_DEBRIS];

static inline void randomize_debris_properties(uint16_t i) {
    fx3_t r;
    r.x = fx_random_signed_one();
    r.y = fx_random_signed_one();
    r.z = fx_random_signed_one();
    fx3_normalize_ip(&r);
    debris_rotation_axes[i] = r;

    debris_speeds[i] = (fx_random_one() >> 12) + (FX_ONE >> 12);
}

static void init_debris() {
    uint16_t i;
    for (i = 0; i < NUM_DEBRIS; ++i) {
        fx3_t r;
        r.x = random_debris_x();
        r.y = random_debris_x();
        r.z = random_debris_x();
        debris_positions[i] = r;
        randomize_debris_properties(i);
    }
}

void restart() {
    timing.current_frame = 0;
    timing.tick_accumulator = 0;
    timing.current_tick = 0;
    timing.ticks_to_advance = 0;

    xorshift32_reset();
    init_debris();
}

//

#define BENCHMARK_DURATION_FRAMES 64
#define BENCHMARK_STEP_TICKS 32

static inline int is_benchmark_running() {
    return benchmark_timer != 0;
}

static inline void start_benchmark() {
    aw_assert(!is_benchmark_running());
    benchmark_timer = 1;
    benchmark_start_tick = read_timer_ticks();
    restart();
}

static inline void end_benchmark() {
    uint32_t num_ticks = read_timer_ticks() - benchmark_start_tick;
    benchmark_result = num_ticks ? fx_rcp(num_ticks << 10) : 0;
    benchmark_timer = 0;
    restart();
}

//

static void update_timing(uint32_t dt_us) {
    timing.current_frame++;

    if (is_benchmark_running()) {
        if (benchmark_timer == BENCHMARK_DURATION_FRAMES) {
            end_benchmark();
        } else {
            benchmark_timer++;
            timing.ticks_to_advance = BENCHMARK_STEP_TICKS;
        }
    } else {
        timing.ticks_to_advance = 0;
        timing.tick_accumulator += dt_us;
        while (timing.tick_accumulator >= TICK_LENGTH_US) {
            timing.tick_accumulator -= TICK_LENGTH_US;
            timing.ticks_to_advance++;
        }
    }

    timing.current_tick += timing.ticks_to_advance;
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
                debris_enabled ^= 1;
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

            case 'b':
                if (!is_benchmark_running())
                    start_benchmark();
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

static void update_debris() {
    uint16_t i;
    for (i = 0; i < NUM_DEBRIS; ++i) {
        debris_positions[i].y -= debris_speeds[i];
        if (debris_positions[i].y <= -2048) {
            debris_positions[i].y += 4096;
            debris_positions[i].x = random_debris_x();
            debris_positions[i].z = random_debris_x();
            randomize_debris_properties(i);
        }
    }
}

static void update() {
    uint16_t i = 0;
    for (i = 0; i < timing.ticks_to_advance; ++i) {
        update_debris();
    }
}

static inline void split_number(uint16_t v, uint8_t* o, uint8_t* te, uint8_t* h, uint8_t* th) {
    uint8_t ones, tens, hundreds, thousands;
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
        mov cx, 1000
        xor dx, dx
        div cx
        mov thousands, al
        mov ax, dx
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
    *o = ones;
    *te = tens;
    *h = hundreds;
    *th = thousands;
}

static inline char number_to_char(uint8_t v) {
    return '0' + v;
}

static inline void number_to_string(char* buf, uint16_t v) {
    uint8_t ones, tens, hundreds, thousands;
    split_number(v, &ones, &tens, &hundreds, &thousands);
    buf[0] = thousands ? number_to_char(thousands) : ' ';
    buf[1] = (thousands || hundreds) ? number_to_char(hundreds) : ' ';
    buf[2] = (thousands || hundreds || tens) ? number_to_char(tens) : ' ';
    buf[3] = number_to_char(ones);
}

static void draw_fps() {
    char buf[6] = { 0 };
    uint16_t v = minu32(fps.average, 9999);
    buf[0] = vsync ? '*' : ' ';
    number_to_string(buf + 1, v);
    draw_text(buf, 320 - 24, 4, 4);
}

static const fx3_t axis_x = { FX_ONE, 0, 0 };
static const fx3_t axis_y = { 0, FX_ONE, 0 };
static const fx3_t axis_z = { 0, 0, FX_ONE };

static void setup_view_matrix(fx4x3_t* view_matrix) {
    fx_t rotation_angle_x = fx_sin(-10000 + (fx_t)timing.current_tick * 27) / 12;
    fx_t rotation_angle_z = 25000 + (fx_t)timing.current_tick * 40;
    fx4_t view_rotation_x;
    fx4_t view_rotation_z;
    fx4_t view_rotation;
    fx3_t view_translation = { 0, 0, 1400 };

    fx_quat_rotation_axis_angle(&view_rotation_x, &axis_x, -FX_ONE / 4 + rotation_angle_x);
    fx_quat_rotation_axis_angle(&view_rotation_z, &axis_z, rotation_angle_z);
    fx_quat_mul(&view_rotation, &view_rotation_x, &view_rotation_z);

    fx4x3_rotation_translation(view_matrix, &view_rotation, &view_translation);
}

#define NUM_STARS 128
static int8_t star_positions[NUM_STARS * 3];

static void init_stars() {
    int8_t* p_tgt = star_positions;
    uint16_t i, j;
    for (i = 0; i < NUM_STARS; ++i) {
        int8_t x, y, z;

        for (;;) {
            int ok = 1;
            fx3_t v;
            v.x = fx_random_signed_one();
            v.y = fx_random_signed_one();
            v.z = fx_random_signed_one();
            fx3_normalize_ip(&v);

            x = clamp16(v.x >> 9, INT8_MIN, INT8_MAX);
            y = clamp16(v.y >> 9, INT8_MIN, INT8_MAX);
            z = clamp16(v.z >> 9, INT8_MIN, INT8_MAX);

            {
                int8_t* p_iter = star_positions;
                for (j = 0; j < i; ++j) {
                    int8_t u, v, w;
                    u = *p_iter++;
                    v = *p_iter++;
                    w = *p_iter++;
                    if (x == u && y == v && z == w) {
                        ok = 0;
                        break;
                    }
                }
            }

            if (ok)
                break;
        }

        *p_tgt++ = x;
        *p_tgt++ = y;
        *p_tgt++ = z;
    }
}

static void draw_stars(const fx4x3_t* view_matrix) {
    int16_t x, y;
    uint16_t i;
    fx3_t v;
    int8_t* p_iter = star_positions;

    for (i = 0; i < NUM_STARS; ++i) {
        v.x = (*p_iter++) << 9;
        v.y = (*p_iter++) << 9;
        v.z = (*p_iter++) << 9;
        fx_transform_vector_ip(view_matrix, &v);

        if (v.z < NEAR_CLIP)
            continue;

        project_to_screen(&v);

        x = v.x >> RASTER_SUBPIXEL_BITS;
        y = v.y >> RASTER_SUBPIXEL_BITS;

        switch (i & 7) {
            case 0:
            case 1: {
                if (x >= 0 && x <= SCREEN_X_MAX && y >= 0 && y <= SCREEN_Y_MAX) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset] = 117;
                }

                break;
            }

            case 2:
            case 3:
            case 4: {
                if (x >= 0 && x <= SCREEN_X_MAX && y >= 0 && y <= SCREEN_Y_MAX) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset] = 9;
                }

                break;
            }

            case 5:
            case 6: {
                if (x >= 1 && x <= SCREEN_X_MAX - 1 && y >= 1 && y <= SCREEN_Y_MAX - 1) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset - SCREEN_WIDTH] = 8;
                    dblbuf[offset - 1] = 8;
                    dblbuf[offset] = 117;
                    dblbuf[offset + 1] = 8;
                    dblbuf[offset + SCREEN_WIDTH] = 8;
                }

                break;
            }

            case 7: {
                if (x >= 1 && x < SCREEN_X_MAX - 1 && y >= 1 && y < SCREEN_Y_MAX - 1) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
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
static fx4x3_t scrap_mesh_adjust_matrix;

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

static void draw_scrap(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &scrap_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        scrap_num_indices, scrap_num_vertices,
        scrap_indices, scrap_face_colors, scrap_vertices);
}

#define DEBRIS_CLIP_BORDER 32
#define DEBRIS_LEFT_CLIP (-DEBRIS_CLIP_BORDER << RASTER_SUBPIXEL_BITS)
#define DEBRIS_RIGHT_CLIP ((SCREEN_WIDTH + DEBRIS_CLIP_BORDER) << RASTER_SUBPIXEL_BITS)
#define DEBRIS_TOP_CLIP (-DEBRIS_CLIP_BORDER << RASTER_SUBPIXEL_BITS)
#define DEBRIS_BOTTOM_CLIP ((SCREEN_HEIGHT + DEBRIS_CLIP_BORDER) << RASTER_SUBPIXEL_BITS)

static void draw_debris(const fx4x3_t* view_matrix) {
    fx4_t rotation;
    fx_t rotation_angle;
    fx3_t transformed_position;
    const fx3_t* position_iter = debris_positions;
    uint16_t i;
    for (i = 0; i < NUM_DEBRIS; ++i) {
        const fx3_t* position = position_iter++;
        fx_transform_point(&transformed_position, view_matrix, position);
        if (transformed_position.z < NEAR_CLIP)
            continue;

        project_to_screen(&transformed_position);
        if (transformed_position.x < DEBRIS_LEFT_CLIP ||
            transformed_position.x > DEBRIS_RIGHT_CLIP ||
            transformed_position.y < DEBRIS_TOP_CLIP ||
            transformed_position.y > DEBRIS_BOTTOM_CLIP)
            continue;

        rotation_angle = position->y * 80;
        fx_quat_rotation_axis_angle(&rotation, &debris_rotation_axes[i], rotation_angle);

        if (i < NUM_DEBRIS / 4) {
            draw_scrap(view_matrix, &rotation, position);
        } else {
            draw_asteroid(view_matrix, &rotation, position);
        }
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

    if (debris_enabled)
        draw_debris(&view_matrix);

    flush_mesh_draw_buffer(draw_mode);

    if (!is_benchmark_running()) {
        if (texts_enabled) {
            if (help) {
                int16_t y = 4;
                draw_text("build 2023-09-29", 4, y, 4); y += 12;
                draw_text("Sorry, this is nonplayable.", 4, y, 4); y += 12;
                draw_text("v: Toggle vertical sync", 4, y, 4); y += 6;
                draw_text("w: Change draw mode (solid/wireframe)", 4, y, 4); y += 6;
                draw_text("b: Run benchmark (result is shown in blue next to fps)", 4, y, 4); y += 6;
                draw_text("1-5: Toggle rendering (stars, debris, ship, texts, fps)", 4, y, 4); y += 6;
                draw_text("esc: Exit to DOS", 4, y, 4); y += 12;
                draw_text("Made for DOS COM Jam 2023", 4, y, 4); y += 6;
                draw_text("https://aarnig.itch.io/dos-com-jam", 4, y, 4); y += 6;
                draw_text("aarni.gratseff@gmail.com", 4, y, 4); y += 6;
            } else {
                draw_texts();
            }
        }

        if (fps_enabled) {
            if (benchmark_result) {
                char buf[5] = { 0 };
                number_to_string(buf, minu32(benchmark_result, 9999));
                draw_text(buf, 320 - 48, 4, 8);
            }

            draw_fps();
        }
    }
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

    fx4x3_identity(&scrap_mesh_adjust_matrix);
    scrap_mesh_adjust_matrix.m[FX4X3_00] = scrap_size.x << 1;
    scrap_mesh_adjust_matrix.m[FX4X3_11] = scrap_size.y << 1;
    scrap_mesh_adjust_matrix.m[FX4X3_22] = scrap_size.z << 1;
    scrap_mesh_adjust_matrix.m[FX4X3_30] = scrap_center.x >> 7;
    scrap_mesh_adjust_matrix.m[FX4X3_31] = scrap_center.y >> 7;
    scrap_mesh_adjust_matrix.m[FX4X3_32] = scrap_center.z >> 7;

    init_stars();
    init_debris();

    //

    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf)
        goto exit;

    tm_buffer = (int16_t __far*)_fmalloc(sizeof(int16_t) * TM_BUFFER_SIZE);
    if (!tm_buffer)
        goto exit;

    draw_buffer = (int16_t __far*)_fmalloc(sizeof(int16_t) * 8 * DRAW_BUFFER_MAX_TRIANGLES);
    if (!draw_buffer)
        goto exit;

    sort_buffer = (uint32_t __far*)_fmalloc(sizeof(uint32_t) * DRAW_BUFFER_MAX_TRIANGLES);
    if (!sort_buffer)
        goto exit;

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

        frame_start_ticks = read_timer_ticks();

        if (previous_frame_ticks == 0)
            previous_frame_ticks = frame_start_ticks;

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

            mov eax, 0x08080808

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

        if (vsync && !is_benchmark_running())
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
