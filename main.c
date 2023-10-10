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

#include "keys.h"
#include "vga.h"
#include "minmax.h"
#include "util.h"
#include "pal.h"
#include "song.h"

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

//

static uint8_t timer_initialized = 0;

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

static uint8_t keyboard_initialized = 0;

// defined in keyb.asm
void keyb_init();
void keyb_cleanup();

//

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

static volatile uint8_t music_enabled = 1;
static volatile uint8_t music_stopped = 0;
static volatile uint16_t music_volume = 256;
static volatile uint8_t sfx_enabled = 1;
static volatile uint8_t sfx_stopped = 0;
static volatile uint16_t sfx_volume = 256;

static uint8_t benchmark_timer = 0;
static uint32_t benchmark_start_tick = 0;
static uint32_t benchmark_result = 0;

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

#define TICK_LENGTH_US (1000000UL / 120)

static struct {
    uint32_t current_frame;
    uint32_t tick_accumulator;
    uint32_t current_tick;
    uint16_t ticks_to_advance;
} timing = { 0 };

static struct {
    uint16_t targeting_timer;
    fx3_t target_position;
} game_state = { 0 };

//

static void restart() {
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

static void update() {
    uint16_t i = 0;
    for (i = 0; i < timing.ticks_to_advance; ++i) {
        update_debris();
    }

    game_state.target_position = debris_positions[0];
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
    fx3_t eye, target;
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

    fx4x3_look_at(view_matrix, &eye, &target);
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

#if 0
    if (p.x - TARGETING_BOX_HALF_WIDTH < 0 ||
        p.x + TARGETING_BOX_HALF_WIDTH >= SCREEN_WIDTH ||
        p.y - TARGETING_BOX_HALF_HEIGHT < 0 ||
        p.y + TARGETING_BOX_HALF_HEIGHT >= SCREEN_HEIGHT)
        return;
#endif

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
    draw_text("b: Run benchmark (result is shown in blue next to fps)", 4, y, 4); y += 6;
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
    x = draw_text(buffer, x, 200 - (6 + 6), 4);

    if (((t >> 1) & 1) == 0)
        draw_text_cursor(x, 200 - (6 + 6), 7);
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
                draw_help();
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

#if 0
        {
            uint16_t i;
            for (i = 0; i < NUM_DEBRIS; ++i) {
                draw_targeting(&view_matrix, &debris_positions[i]);
            }
        }

        draw_darkened_box(8, 120, 120, 200 - 8);
#endif
    }
}

static void update_music() {
    static uint32_t time_accumulator = 0;
    static uint32_t next_delta_time = 1000000UL;
    static uint16_t event_index = 0;
    static uint8_t channel_programs[NUM_OPL_MUSIC_CHANNELS] = { 0 };
    uint8_t v;

#if defined(INLINE_ASM)
    _asm {
        .386
        add dword ptr time_accumulator, TIMER_TICK_USEC
    }
#else
    time_accumulator += TIMER_TICK_USEC;
#endif

    while (time_accumulator >= next_delta_time) {
        time_accumulator -= next_delta_time;

        v = song_events[event_index++];
        if (v == 255) {
            event_index = 0;
            next_delta_time = 0;
        } else if (v & 0x80) {
            uint8_t v2 = song_events[event_index++];
            uint8_t v3 = song_events[event_index++];
            next_delta_time = ((uint32_t)(v & ~0x80) << 16) | ((uint32_t)v2 << 8) | v3;
            next_delta_time <<= 3;
        } else if (v & 0x40) {
            uint8_t v2 = song_events[event_index++];
            next_delta_time = ((uint32_t)(v & ~0x40) << 8) | v2;
            next_delta_time <<= 3;
        } else if (v & 0x20) {
            channel_programs[v & ~0x20] = song_events[event_index++];
            next_delta_time = 0;
        } else if (v & 0x10) {
            opl_stop(v & ~0x10);
            next_delta_time = 0;
        } else {
            uint8_t note, velocity;
            aw_assert(v < NUM_OPL_MUSIC_CHANNELS);
            note = song_events[event_index++];
            velocity = song_events[event_index++];
            velocity = ((uint16_t)velocity * music_volume) >> 8;
            opl_play(v, channel_programs[v], note, velocity);
            next_delta_time = 0;
        }
    }
}

#define MAX_SFX_EVENTS 16

typedef struct sfx_event_t {
    uint8_t type;
    uint32_t duration;
    uint32_t time_left;
} sfx_event_t;

static volatile sfx_event_t sfx_events[MAX_SFX_EVENTS] = { 0 };

typedef struct sfx_event_type_t {
    uint32_t duration;
    uint32_t program;
    uint32_t note;
    uint32_t velocity;
} sfx_event_type_t;

#define NUM_SFX_EVENT_TYPES 2

static const sfx_event_type_t sfx_event_types[] = {
    { 60000, 0, 0, 0 },
    { 60000, 80, 69, 24 },
    { 80000, 80, 57, 24 },
};

static const sfx_event_type_t* get_sfx_event_type_data(uint8_t type) {
    aw_assert(type != 0 && type < NUM_SFX_EVENT_TYPES + 1);
    return &sfx_event_types[type - 1];
}

static void update_sfx() {
    uint8_t i;
    volatile sfx_event_t* e = &sfx_events[0];
    if (e->time_left != 0) {
        const sfx_event_type_t* t = get_sfx_event_type_data(e->type);
        if (e->time_left >= TIMER_TICK_USEC) {
            if (e->time_left == e->duration) {
                if (t->velocity != 0)
                    opl_play(OPL_SFX_CHANNEL, t->program, t->note, t->velocity);
            }

            e->time_left -= TIMER_TICK_USEC;
        } else {
            if (t->velocity != 0)
                opl_stop(OPL_SFX_CHANNEL);

            e->type = 0;
            e->time_left = 0;
        }
    }

    if (e->time_left == 0) {
        for (i = 0; i < MAX_SFX_EVENTS - 1; ++i) {
            sfx_events[i] = sfx_events[i + 1];
        }

        sfx_events[MAX_SFX_EVENTS - 1].type = 0;
    }
}

static inline void push_sfx_event(uint8_t type) {
    uint8_t i;
    for (i = 0; i < MAX_SFX_EVENTS; ++i) {
        if (!sfx_events[i].type) {
            sfx_events[i].type = type;
            sfx_events[i].duration = get_sfx_event_type_data(type)->duration;
            sfx_events[i].time_left = sfx_events[i].duration;
            break;
        }
    }
}

static inline void sfx_blip() {
    _disable();
    push_sfx_event(1);
    push_sfx_event(2);
    push_sfx_event(1);
    push_sfx_event(2);
    push_sfx_event(1);
    push_sfx_event(2);
    _enable();
}

static inline void sfx_blip_error() {
    _disable();
    push_sfx_event(1);
    push_sfx_event(2);
    _enable();
}

void timer_update() {
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

static void update_input() {
    if (kbhit()) {
        char ch = getch();
        switch (ch) {
            case 27:
                quit = 1;
                break;

            case 'a':
                sfx_blip();
                break;

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

            case 'h':
                help ^= 1;
                break;

            default:
                break;
        }
    }
}

#define INVALID_KEY 0xff

static volatile int extended_key = 0;

static uint8_t translate_key(uint32_t code) {
    switch (code) {
        case 0x01: return KEY_ESC;
        case 0x1c: return KEY_ENTER;
        case 0x39: return KEY_SPACE;
        case 0x0e: return KEY_BACKSPACE;
        case 0x0f: return KEY_TAB;

        case 0x2a: return KEY_LSHIFT;
        case 0x36: return KEY_RSHIFT;
        case 0x1d: return KEY_LCTRL;
        case 0x38: return KEY_LALT;

        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4b: return KEY_LEFT;
        case 0x4d: return KEY_RIGHT;

        case 0x02: return KEY_1;
        case 0x03: return KEY_2;
        case 0x04: return KEY_3;
        case 0x05: return KEY_4;
        case 0x06: return KEY_5;
        case 0x07: return KEY_6;
        case 0x08: return KEY_7;
        case 0x09: return KEY_8;
        case 0x0a: return KEY_9;
        case 0x0b: return KEY_0;

        case 0x1e: return KEY_A;
        case 0x30: return KEY_B;
        case 0x2e: return KEY_C;
        case 0x20: return KEY_D;
        case 0x12: return KEY_E;
        case 0x21: return KEY_F;
        case 0x22: return KEY_G;
        case 0x23: return KEY_H;
        case 0x17: return KEY_I;
        case 0x24: return KEY_J;
        case 0x25: return KEY_K;
        case 0x26: return KEY_L;
        case 0x32: return KEY_M;
        case 0x31: return KEY_N;
        case 0x18: return KEY_O;
        case 0x19: return KEY_P;
        case 0x10: return KEY_Q;
        case 0x13: return KEY_R;
        case 0x1f: return KEY_S;
        case 0x14: return KEY_T;
        case 0x16: return KEY_U;
        case 0x2f: return KEY_V;
        case 0x11: return KEY_W;
        case 0x2d: return KEY_X;
        case 0x15: return KEY_Y;
        case 0x2c: return KEY_Z;

        case 0x3b: return KEY_F1;
        case 0x3c: return KEY_F2;
        case 0x3d: return KEY_F3;
        case 0x3e: return KEY_F4;
        case 0x3f: return KEY_F5;
        case 0x40: return KEY_F6;
        case 0x41: return KEY_F7;
        case 0x42: return KEY_F8;
        case 0x43: return KEY_F9;
        case 0x44: return KEY_F10;
        case 0x57: return KEY_F11;
        case 0x58: return KEY_F12;

        case 0x29: return KEY_TILDE;

        case 0x33: return KEY_COMMA;
        case 0x34: return KEY_PERIOD;
        case 0x35: return KEY_HYPHEN;
        case 0x2b: return KEY_ASTERISK;
        case 0x0c: return KEY_PLUS;
        case 0x56: return KEY_ANGLE_BRACKETS;

        case 0x11c: return KEY_KEYPAD_ENTER;

        case 0x11d: return KEY_RCTRL;
        case 0x138: return KEY_RALT;

        case 0x148: return KEY_GREY_UP;
        case 0x150: return KEY_GREY_DOWN;
        case 0x14b: return KEY_GREY_LEFT;
        case 0x14d: return KEY_GREY_RIGHT;

        case 0x152: return KEY_INSERT;
        case 0x153: return KEY_DELETE;
        case 0x147: return KEY_HOME;
        case 0x14f: return KEY_END;
        case 0x149: return KEY_PAGE_UP;
        case 0x151: return KEY_PAGE_DOWN;

        default: return INVALID_KEY;
    }
}

#define KEY_BUFFER_SIZE 256
static volatile uint8_t key_buffer[KEY_BUFFER_SIZE];
static volatile uint32_t key_buffer_position = 0;

void keyb_key(uint8_t code) {
    if (code != 0xe0) {
        uint8_t key = translate_key((code & 0x7f) | (extended_key ? 0x100 : 0));
        if (key != INVALID_KEY) {
            int up = (code & 0x80) == 0x80;
            aw_assert(key_buffer_position < KEY_BUFFER_SIZE);
            key_buffer[key_buffer_position] = key | (up ? KEY_UP_FLAG : 0);
            key_buffer_position = (key_buffer_position + 1) & (KEY_BUFFER_SIZE - 1);

            if (key == KEY_ESC)
                quit = 1;
        }

        extended_key = 0;
    } else {
        extended_key = 1;
    }
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
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;
            vga_set_palette(i, r >> 2, g >> 2, b >> 2);
            vga_set_palette(i + 128, r >> 4, g >> 4, b >> 4);
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

        update_fps(frame_dt);
        update_timing(frame_dt);
        update_input();
        update();

        dblbuf_clear();

        draw();

        if (vsync && !is_benchmark_running())
            vga_wait_for_retrace();

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
