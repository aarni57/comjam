#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

//

#if defined(__WATCOMC__)
#   define INLINE_ASM
#endif

#if 0
#   define BENCHMARK_ENABLED
#endif

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

#include "timer.h"
#include "keyb.h"
#include "vga.h"
#include "minmax.h"
#include "util.h"
#include "pal.h"

//

#include "dblbuf.h" // Needs to be before drawing code
#include "meshdraw.h"
#include "drawtext.h"
#include "drawutil.h"
#include "randutil.h"
#include "debris.h"
#include "stars.h"

//

static int opl = 0;
#include "opl.h"

#include "sfx.h"
#include "music.h"

//

static volatile uint8_t music_enabled = 1;
static volatile uint8_t sfx_enabled = 1;

void timer_update() {
    static uint8_t music_stopped = 0;
    static uint8_t sfx_stopped = 0;

    if (opl) {
        if (music_enabled) {
            update_music();
            music_stopped = 0;
        } else if (!music_stopped) {
            uint8_t i;
            for (i = 0; i < NUM_OPL_MUSIC_CHANNELS; ++i)
                opl_stop(i);

            music_stopped = 1;
        }

        if (sfx_enabled) {
            update_sfx();
            sfx_stopped = 0;
        } else {
            opl_stop(OPL_SFX_CHANNEL);
            sfx_stopped = 1;
        }
    }
}

//

static uint8_t timer_initialized = 0;
static uint8_t keyboard_initialized = 0;

static const char* exit_message = "JEMM unloaded... Not really :-)";

static uint8_t quit = 0;
static uint8_t vsync = 0;
static uint8_t help = 0;
static uint8_t draw_mode = 0;
static uint8_t stars_enabled = 1;
static uint8_t debris_enabled = 1;
static uint8_t ship_enabled = 1;
static uint8_t texts_enabled = 1;
static uint8_t fps_enabled = 1;

#if defined(BENCHMARK_ENABLED)
static uint8_t benchmark_timer = 0;
static uint32_t benchmark_start_tick = 0;
static uint32_t benchmark_result = 0;
#endif

//

static struct {
    uint32_t time_accumulator;
    uint32_t frame_accumulator;
    uint32_t average;
} fps = { 0 };

static void update_fps(uint32_t frame_dt) {
    fps.time_accumulator += frame_dt;
    if (fps.time_accumulator >= 1000000) {
        fps.average = div32(mul32(fps.frame_accumulator, 1000000), fps.time_accumulator);
        fps.time_accumulator = 0;
        fps.frame_accumulator = 0;
    }

    fps.frame_accumulator++;
}

//

#define TICK_LENGTH_US (1000000UL / 128)

static struct {
    uint32_t current_frame;
    uint32_t tick_accumulator;
    uint32_t current_tick;
    uint16_t ticks_to_advance;
} timing = { 0 };

static struct {
    uint16_t targeting_timer;
    fx3_t target_position;

    fx3_t forward;
    fx3_t up;

    fx_t yaw_speed;
    fx_t pitch_speed;

    fx3_t linear_velocity;

    uint16_t set_speed;
    uint16_t actual_speed;

    uint8_t turning_left;
    uint8_t turning_right;
    uint8_t turning_up;
    uint8_t turning_down;

    uint8_t accelerating;
    uint8_t decelerating;
} gameplay = { 0 };

static void init_gameplay() {
    memset(&gameplay, 0, sizeof(gameplay));
    gameplay.forward.y = FX_ONE;
    gameplay.up.z = FX_ONE;
}

//

static void restart() {
    timing.current_frame = 0;
    timing.tick_accumulator = 0;
    timing.current_tick = 0;
    timing.ticks_to_advance = 0;

    xorshift32_reset();
    init_debris();
    init_gameplay();
}

//

#if defined(BENCHMARK_ENABLED)

#define BENCHMARK_DURATION_FRAMES 128
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
    benchmark_result = num_ticks ? fx_rcp(num_ticks << 9) : 0;
    benchmark_timer = 0;
    restart();
}

static void update_benchmark() {
    fx3_t zero_vec = { 0 };
    fx3_t debris_movement = { 0, -100, 0 };
    uint16_t i = 0;
    for (i = 0; i < timing.ticks_to_advance; ++i) {
        update_debris(&zero_vec, &debris_movement);
    }
}

static void draw_benchmark() {
    fx4x3_t view_matrix;

    {
        fx3_t eye, target, up;
        fx_t t1, t2, s1, c1, s2;

        t1 = 25000 + (fx_t)timing.current_tick * 20;
        t2 = fx_sin(-10000 + (fx_t)timing.current_tick * 13) / 12;

        s1 = fx_sin(t1);
        c1 = fx_cos(t1);
        s2 = fx_sin(t2);

        eye.x = s1;
        eye.y = c1;
        eye.z = s2;
        fx3_normalize_ip(&eye);
        eye.x >>= 2;
        eye.y >>= 2;
        eye.z >>= 2;

        target.x = 0;
        target.y = 0;
        target.z = 0;

        up.x = 0;
        up.y = 0;
        up.z = FX_ONE;

        fx4x3_look_at(&view_matrix, &eye, &target, &up);
    }

    draw_stars(&view_matrix);
    draw_debris(&view_matrix);

    {
        fx4_t rotation = { 0, 0, 0, FX_ONE };
        fx3_t translation = { 0, 0, 0 };
        draw_ship(&view_matrix, &rotation, &translation);
    }

    flush_mesh_draw_buffer(draw_mode);
}

#else

static inline int is_benchmark_running() { return 0; }
static inline void update_benchmark() {}
static inline void draw_benchmark() {}

#endif

//

static void update_timing(uint32_t dt_us) {
    timing.current_frame++;

    if (is_benchmark_running()) {
#if defined(BENCHMARK_ENABLED)
        if (benchmark_timer == BENCHMARK_DURATION_FRAMES) {
            end_benchmark();
        } else {
            benchmark_timer++;
            timing.ticks_to_advance = BENCHMARK_STEP_TICKS;
        }
#endif
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

#define TURNING_ACCELERATION 500
#define TURNING_SPEED_DAMPENING 62800
#define MAX_TURNING_SPEED FX_ONE
#define TURNING_SPEED_LOW_LIMIT 64

#define LINEAR_ACCELERATION 1400

#define GAMEPLAY_SPEED_TO_WORLD_MUL FX_ONE * 128
#define GAMEPLAY_SPEED_FROM_WORLD_MUL FX_ONE / 128

#define MAX_GAMEPLAY_SPEED 300
#define GAMEPLAY_SPEED_STEP 2

static const fx4_t IDENTITY_QUAT = { 0, 0, 0, FX_ONE };

static void update() {
    uint16_t i = 0;
    for (i = 0; i < timing.ticks_to_advance; ++i) {
        {
            uint16_t previous_set_speed = gameplay.set_speed;

            if (gameplay.accelerating && gameplay.set_speed < MAX_GAMEPLAY_SPEED)
                gameplay.set_speed += GAMEPLAY_SPEED_STEP;

            if (gameplay.decelerating && gameplay.set_speed != 0)
                gameplay.set_speed -= GAMEPLAY_SPEED_STEP;

            if (gameplay.set_speed != previous_set_speed &&
                (gameplay.set_speed % 20) == 0) {
                    sfx_short_scrub();
            }
        }

        {
            fx3_t right;
            fx4_t yaw_rotation, pitch_rotation;

            fx3_cross(&right, &gameplay.forward, &gameplay.up);

            gameplay.yaw_speed = fx_mul(gameplay.yaw_speed, TURNING_SPEED_DAMPENING);
            gameplay.pitch_speed = fx_mul(gameplay.pitch_speed, TURNING_SPEED_DAMPENING);

            if (gameplay.turning_left)
                gameplay.yaw_speed += TURNING_ACCELERATION;

            if (gameplay.turning_right)
                gameplay.yaw_speed -= TURNING_ACCELERATION;

            if (gameplay.turning_up)
                gameplay.pitch_speed += TURNING_ACCELERATION;

            if (gameplay.turning_down)
                gameplay.pitch_speed -= TURNING_ACCELERATION;

            gameplay.yaw_speed = fx_clamp(gameplay.yaw_speed, -MAX_TURNING_SPEED, MAX_TURNING_SPEED);
            gameplay.pitch_speed = fx_clamp(gameplay.pitch_speed, -MAX_TURNING_SPEED, MAX_TURNING_SPEED);

            if (fx_abs(gameplay.yaw_speed) > TURNING_SPEED_LOW_LIMIT)
                fx_quat_rotation_axis_angle(&yaw_rotation, &gameplay.up, gameplay.yaw_speed >> 7);
            else
                yaw_rotation = IDENTITY_QUAT;

            if (fx_abs(gameplay.pitch_speed) > TURNING_SPEED_LOW_LIMIT)
                fx_quat_rotation_axis_angle(&pitch_rotation, &right, gameplay.pitch_speed >> 7);
            else
                pitch_rotation = IDENTITY_QUAT;

            {
                fx3x3_t rm;
                fx4_t r;

                fx_quat_mul(&r, &yaw_rotation, &pitch_rotation);
                fx4_normalize_ip(&r);

                fx3x3_rotation(&rm, &r);

                fx_transform_vector_ip(&rm, &gameplay.forward);
                fx3_normalize_ip(&gameplay.forward);

                fx_transform_vector_ip(&rm, &gameplay.up);
                fx3_normalize_ip(&gameplay.up);
            }

            fx3_mul_ip(&gameplay.linear_velocity, FX_ONE * 97 / 100);

            if (gameplay.set_speed != 0) {
                fx_t actual_speed = fx3_length(&gameplay.linear_velocity);
                fx_t target_speed = fx_mul(gameplay.set_speed, GAMEPLAY_SPEED_TO_WORLD_MUL);
                if (actual_speed < target_speed) {
                    fx3_t acceleration;
                    fx3_mul(&acceleration, &gameplay.forward, fx_min(target_speed - actual_speed, LINEAR_ACCELERATION));
                    fx3_add_ip(&gameplay.linear_velocity, &acceleration);
                }
            }

            gameplay.actual_speed = fx_mul(fx3_length(&gameplay.linear_velocity), GAMEPLAY_SPEED_FROM_WORLD_MUL);
        }

        {
            fx3_t camera_target, debris_movement;

            camera_target.x = gameplay.forward.x >> 1;
            camera_target.y = gameplay.forward.y >> 1;
            camera_target.z = gameplay.forward.z >> 1;

            debris_movement.x = -gameplay.linear_velocity.x >> 8;
            debris_movement.y = -gameplay.linear_velocity.y >> 8;
            debris_movement.z = -gameplay.linear_velocity.z >> 8;

            update_debris(&camera_target, &debris_movement);
        }
    }

    gameplay.target_position = debris_positions[0];
}

static void draw_fps() {
    char buf[6] = { 0 };
    uint16_t v = minu32(fps.average, 9999);
    buf[0] = vsync ? '*' : ' ';
    number_to_string(buf + 1, v);
    draw_text2(buf, 320 - 26, 4, 8, 2);
}

#define TARGETING_BOX_HALF_WIDTH 12
#define TARGETING_BOX_HALF_HEIGHT 10

static void draw_targeting(const fx4x3_t* view_matrix, const fx3_t* target_position) {
    fx3_t p;
    fx_transform_point(&p, view_matrix, target_position);
    if (p.z < NEAR_CLIP)
        return;

    project_to_screen(&p);
    projected_position_to_screen(&p);

    draw_box_outline(p.x - TARGETING_BOX_HALF_WIDTH,
        p.y - TARGETING_BOX_HALF_HEIGHT,
        p.x + TARGETING_BOX_HALF_WIDTH,
        p.y + TARGETING_BOX_HALF_HEIGHT,
        1);
}

static void draw_help() {
    int16_t y = 4;
    draw_text("build 2023-09-29", 4, y, 4); y += 12;
    draw_text("Sorry, this is nonplayable.", 4, y, 4); y += 12;
    draw_text("v: Toggle vertical sync", 4, y, 4); y += 6;
    draw_text("w: Change draw mode (solid/wireframe)", 4, y, 4); y += 6;
    draw_text("1-5: Toggle rendering (stars, debris, ship, texts, fps)", 4, y, 4); y += 6;
    draw_text("esc: Exit to DOS", 4, y, 4); y += 12;
    draw_text("Made for DOS COM Jam 2023", 4, y, 4); y += 6;
    draw_text("https://aarnig.itch.io/dos-com-jam", 4, y, 4); y += 6;
    draw_text("aarni.gratseff@gmail.com", 4, y, 4); y += 6;
}

static const char* const TEXT = "2669, Gemini sector, Troy system";

static void draw_texts() {
    char buffer[64];
    int16_t x;
    uint8_t i;
    uint32_t t = timing.current_tick >> 4;

    for (i = 0; i < 64; ++i) {
        uint32_t t_step;

        if (TEXT[i] == 0)
            break;

        if (i == 0) {
            t_step = 20;
        } else {
            t_step = TEXT[i] == ' ' ? 2 : 1;
        }

        if (t < t_step)
            break;

        t -= t_step;

        buffer[i] = TEXT[i];
    }

    buffer[i] = 0;

    x = 6;
    x = draw_text(buffer, x, 200 - (6 + 6), 0);

    if (((t >> 1) & 1) == 0)
        draw_text_cursor(x, 200 - (6 + 6), 3);
}

static void draw_line_mesh(int16_t x, int16_t y) {
    const int8_t* lines = scrap_lines;
    const int8_t* lines_end = lines + ((scrap_num_lines << 2) + scrap_num_lines);
    while (lines < lines_end) {
        int8_t x0, y0, x1, y1;
        uint8_t c;
        x0 = *lines++;
        y0 = *lines++;
        x1 = *lines++;
        y1 = *lines++;
        c = *lines++;
        draw_line(x + x0, y + y0, x + x1, y + y1, c);
    }
}

#define LINE_SPACING 8

static void draw() {
    fx3_t zero_vec = { 0, 0, 0 };
    fx4x3_t view_matrix;
    fx4x3_look_at(&view_matrix, &zero_vec, &gameplay.forward, &gameplay.up);

    if (stars_enabled)
        draw_stars(&view_matrix);

    if (debris_enabled)
        draw_debris(&view_matrix);

#if 0
    if (ship_enabled) {
        fx4_t ship_rotation;
        fx3_t ship_translation = { 0, 20000, 0 };
        fx3_t rotation_axis = { 0, 0, FX_ONE };

        fx_quat_rotation_axis_angle(&ship_rotation, &rotation_axis, timing.current_tick << 6);

        draw_ship(&view_matrix, &ship_rotation, &ship_translation);
    }
#endif

    flush_mesh_draw_buffer(draw_mode);

    if (texts_enabled) {
        if (help) {
            draw_help();
        } else {
            //draw_texts();
        }
    }

    if (fps_enabled) {
#if defined(BENCHMARK_ENABLED)
        if (benchmark_result) {
            char buf[5] = { 0 };
            number_to_string(buf, minu32(benchmark_result, 9999));
            draw_text(buf, 320 - 48, 4, 8);
        }
#endif

        draw_fps();
    }

#if 0
    {
        uint16_t i;
        for (i = 0; i < NUM_DEBRIS; ++i) {
            draw_targeting(&view_matrix, &debris_positions[i]);
        }
    }
#endif

    {
        int16_t x = 10;
        int16_t y = 122;
        int16_t width = 104;
        int16_t height = 68;

        draw_darkened_box(x, y, x + width, y + height);

        {
            int16_t tx = x + 4;
            int16_t ty = y + 4;

            draw_text2("1. Repair Droid", tx, ty, 4, 2); ty += LINE_SPACING;
            draw_text2("2. Tractor Beam", tx, ty, 4, 2); ty += LINE_SPACING;
            draw_text2("3. Secondary Display", tx, ty, 4, 2); ty += LINE_SPACING;
            draw_text2("4. Back", tx, ty, 4, 2); ty += LINE_SPACING;

        }

        x = SCREEN_WIDTH - 10 - width;
        draw_darkened_box(x, y, x + width, y + height);

        draw_line_mesh(x + width / 2, y + height / 2);
    }

    {
        int16_t left = SCREEN_WIDTH / 2 - 41;
        int16_t right = SCREEN_WIDTH / 2 + 29;
        int16_t y = 122 + 4;
        char buf[5] = { 0 };
        int16_t text_width;

        draw_text2("SET", left, y, 4, 2);
        draw_text2("ACT", right, y, 4, 2);

        y += LINE_SPACING;

        number_to_string(buf, gameplay.set_speed);
        text_width = calc_text_width(buf);
        draw_text2(buf, left + 5 - (text_width >> 1), y, 4, 2);

        number_to_string(buf, gameplay.actual_speed);
        text_width = calc_text_width(buf);
        draw_text2(buf, right + 5 - (text_width >> 1), y, 4, 2);
    }
}

static void update_input() {
    static uint8_t i = 0;
    uint8_t current_position = key_buffer_position;

    while (i != current_position) {
        switch (key_buffer[i]) {
            case KEY_GREY_LEFT: gameplay.turning_left = 1; break;
            case KEY_GREY_LEFT | KEY_UP_FLAG: gameplay.turning_left = 0; break;
            case KEY_GREY_RIGHT: gameplay.turning_right = 1; break;
            case KEY_GREY_RIGHT | KEY_UP_FLAG: gameplay.turning_right = 0; break;
            case KEY_GREY_UP: gameplay.turning_up = 1; break;
            case KEY_GREY_UP | KEY_UP_FLAG: gameplay.turning_up = 0; break;
            case KEY_GREY_DOWN: gameplay.turning_down = 1; break;
            case KEY_GREY_DOWN | KEY_UP_FLAG: gameplay.turning_down = 0; break;

            case KEY_PLUS: gameplay.accelerating = 1; break;
            case KEY_PLUS | KEY_UP_FLAG: gameplay.accelerating = 0; break;
            case KEY_HYPHEN: gameplay.decelerating = 1; break;
            case KEY_HYPHEN | KEY_UP_FLAG: gameplay.decelerating = 0; break;

            case KEY_ESC:
                quit = 1;
                break;

            case KEY_1:
                sfx_short_blip();
                break;

            case KEY_2:
                sfx_select();
                break;

            case KEY_3:
                sfx_back();
                break;

            case KEY_4:
                sfx_processing();
                break;

            case KEY_5:
                sfx_error();
                break;

            case KEY_V:
                vsync ^= 1;
                break;

#if defined(BENCHMARK_ENABLED)
            case KEY_B:
                start_benchmark();
                break;
#endif

            case KEY_H:
                help ^= 1;
                break;

            default:
                break;
        }

        i = (i + 1) & (KEY_BUFFER_SIZE - 1);
    }

#if 0
        switch (ch) {
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

            case '6':
                music_enabled ^= 1;
                break;

            case 'w':
                draw_mode ^= 1;
                break;

            case 'h':
                help ^= 1;
                break;

            default:
                break;
        }
    }
#endif
}

void main() {
    aw_assert(ship_num_indices % 3 == 0);
    aw_assert(sizeof(ship_vertices) == ship_num_vertices * 3);
    aw_assert(sizeof(ship_indices) == ship_num_indices * sizeof(uint16_t));
    aw_assert(sizeof(ship_face_colors) == ship_num_indices / 3);

    //

    init_meshes();
    init_stars();
    init_debris();
    init_gameplay();

    //

    if (!dblbuf_allocate())
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

    opl_init();
    opl_warmup();

    timer_init(); timer_initialized = 1;
    keyb_init(); keyboard_initialized = 1;

    vga_set_mode(0x13);

    {
        uint16_t i;
        const uint8_t* src = PALETTE;
        aw_assert(NUM_PALETTE_COLORS <= 128);
        for (i = 0; i < NUM_PALETTE_COLORS; ++i) {
            uint16_t r = *src++;
            uint16_t g = *src++;
            uint16_t b = *src++;

            vga_set_palette(i, r >> 2, g >> 2, b >> 2);

            r = (r << 1) + r;
            g = (g << 1) + g;
            b = (b << 1) + b;
            r >>= 5;
            g >>= 5;
            b >>= 5;

            vga_set_palette(i + 128, r, g, b);
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

        if (!is_benchmark_running())
            update_fps(frame_dt);

        update_timing(frame_dt);

        if (!is_benchmark_running()) {
            update_input();
            update();
        } else {
            update_benchmark();
        }

        dblbuf_clear();

        if (!is_benchmark_running()) {
            draw();
            if (vsync)
                vga_wait_for_retrace();
        } else {
            draw_benchmark();
        }

        dblbuf_copy_to_screen();
    }

exit:
    dblbuf_release();
    _ffree(tm_buffer);
    _ffree(draw_buffer);
    _ffree(sort_buffer);

    vga_set_mode(0x3);

    if (keyboard_initialized) keyb_cleanup();
    if (timer_initialized) timer_cleanup();

    opl_done();

    putz(exit_message);
    set_text_cursor(1, 0);
    kb_clear_buffer();
}
