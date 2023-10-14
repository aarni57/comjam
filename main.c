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

#define OBJECT_TYPE_NONE            0
#define OBJECT_TYPE_SHIP_WRECK      1
#define OBJECT_TYPE_ABANDONED_SHIP  2
#define OBJECT_TYPE_CONTAINER       3
#define OBJECT_TYPE_NAV_POINT       4
#define OBJECT_TYPE_JUMP_POINT      5

//

#include "timer.h"
#include "keyb.h"
#include "vga.h"
#include "minmax.h"
#include "util.h"
#include "pal.h"
#include "mem.h"

//

static uint8_t __far* dblbuf = NULL;

//

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

static uint8_t __far* still_images = NULL;

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

//

typedef struct object_t {
    uint8_t type;
} object_t;

typedef struct object_tm_t {
    fx3_t position;
    fx4_t rotation;
} object_tm_t;

typedef struct object_movement_t {
    fx3_t linear_velocity;
    fx4_t angular_velocity;
} object_movement_t;

#define MAX_OBJECTS 8

static uint8_t num_objects = 0;

static object_t objects[MAX_OBJECTS] = { 0 };
static object_tm_t object_tms[MAX_OBJECTS] = { 0 };
static object_movement_t object_movements[MAX_OBJECTS] = { 0 };

static inline void random_object_position(fx3_t* position) {
    position->x = fx_random_signed_one() << 2;
    position->y = fx_random_signed_one() << 2;
    position->z = fx_random_signed_one() << 2;
}

static inline void random_nav_point_position(fx3_t* position) {
    position->x = fx_random_signed_one() << 6;
    position->y = fx_random_signed_one() << 6;
    position->z = fx_random_signed_one() << 6;
}

static void init_objects(const fx3_t* start_position) {
    uint8_t i;

    num_objects = 6;

    objects[0].type = OBJECT_TYPE_ABANDONED_SHIP;
    objects[1].type = OBJECT_TYPE_CONTAINER;
    objects[2].type = OBJECT_TYPE_SHIP_WRECK;
    objects[3].type = OBJECT_TYPE_CONTAINER;

    objects[4].type = OBJECT_TYPE_NAV_POINT;
    random_nav_point_position(&object_tms[4].position);

    objects[5].type = OBJECT_TYPE_JUMP_POINT;
    random_nav_point_position(&object_tms[5].position);

    for (i = 0; i < num_objects; ++i) {
        switch (objects[i].type) {
            case OBJECT_TYPE_ABANDONED_SHIP:
            case OBJECT_TYPE_SHIP_WRECK:
            case OBJECT_TYPE_CONTAINER: {
                object_tm_t* tm = &object_tms[i];
                object_movement_t* movement = &object_movements[i];

                random_object_position(&tm->position);
                fx3_sub_ip(&tm->position, start_position);

                tm->rotation.x = fx_random_signed_one();
                tm->rotation.y = fx_random_signed_one();
                tm->rotation.z = fx_random_signed_one();
                tm->rotation.w = fx_random_signed_one();
                fx_quat_normalize(&tm->rotation);

                movement->linear_velocity.x = fx_random_signed_one() >> 13;
                movement->linear_velocity.y = fx_random_signed_one() >> 13;
                movement->linear_velocity.z = fx_random_signed_one() >> 13;

                {
                    fx3_t axis;
                    fx_t angular_speed;

                    axis.x = fx_random_signed_one();
                    axis.y = fx_random_signed_one();
                    axis.z = fx_random_signed_one();
                    fx3_normalize_ip(&axis);

                    switch (objects[i].type) {
                        case OBJECT_TYPE_ABANDONED_SHIP: angular_speed = FX_ONE >> 12; break;
                        case OBJECT_TYPE_SHIP_WRECK: angular_speed = FX_ONE >> 11; break;
                        case OBJECT_TYPE_CONTAINER: angular_speed = FX_ONE >> 10; break;
                    }

                    fx_quat_rotation_axis_angle(&movement->angular_velocity, &axis, angular_speed);
                }

                break;
            }

            case OBJECT_TYPE_NAV_POINT:
            case OBJECT_TYPE_JUMP_POINT: {
                object_tm_t* tm = &object_tms[i];
                fx3_sub_ip(&tm->position, start_position);
                break;
            }

            default:
                aw_assert(0);
                break;
        }
    }
}

static void update_objects(const fx3_t* movement) {
    static uint8_t normalize_counter = 0x3f;
    uint8_t i;
    for (i = 0; i < num_objects; ++i) {
        switch (objects[i].type) {
            case OBJECT_TYPE_SHIP_WRECK:
            case OBJECT_TYPE_ABANDONED_SHIP:
            case OBJECT_TYPE_CONTAINER: {
                object_tm_t* tm = &object_tms[i];
                object_movement_t* om = &object_movements[i];

                fx3_add_ip(&tm->position, &om->linear_velocity);
                fx3_add_ip(&tm->position, movement);

                fx_quat_mul_ip(&tm->rotation, &om->angular_velocity);

                if (((normalize_counter + i) & 0x3f) == 0)
                    fx4_normalize_ip(&tm->rotation);

                break;
            }

            default: {
                object_tm_t* tm = &object_tms[i];
                fx3_add_ip(&tm->position, movement);
                break;
            }
        }
    }

    normalize_counter--;
    if (normalize_counter == 0)
        normalize_counter = 0x3f;
}

#define OBJECT_FAR_CLIP (1024L * 1024)
#define OBJECT_BOUNDARY_CHECK_MIN_Z (1024L * 64)
#define OBJECT_CLIP_BORDER 16
#define OBJECT_LEFT_CLIP (-OBJECT_CLIP_BORDER << RASTER_SUBPIXEL_BITS)
#define OBJECT_RIGHT_CLIP ((SCREEN_X_MAX + OBJECT_CLIP_BORDER) << RASTER_SUBPIXEL_BITS)
#define OBJECT_TOP_CLIP (-OBJECT_CLIP_BORDER << RASTER_SUBPIXEL_BITS)
#define OBJECT_BOTTOM_CLIP ((SCREEN_Y_MAX + OBJECT_CLIP_BORDER) << RASTER_SUBPIXEL_BITS)

static void draw_objects(const fx4x3_t* view_matrix) {
    const object_tm_t* tm;
    fx3_t transformed_position;
    uint8_t i;
    for (i = 0; i < num_objects; ++i) {
        tm = &object_tms[i];

        fx_transform_point(&transformed_position, view_matrix, &tm->position);
        if (transformed_position.z < NEAR_CLIP || transformed_position.z > OBJECT_FAR_CLIP)
            continue;

        if (transformed_position.z >= OBJECT_BOUNDARY_CHECK_MIN_Z) {
            project_to_screen(&transformed_position,
                SCREEN_SUBPIXEL_CENTER_X, SCREEN_SUBPIXEL_CENTER_Y);
            if (transformed_position.x < OBJECT_LEFT_CLIP ||
                transformed_position.x > OBJECT_RIGHT_CLIP ||
                transformed_position.y < OBJECT_TOP_CLIP ||
                transformed_position.y > OBJECT_BOTTOM_CLIP)
                continue;
        }

        draw_object(view_matrix, &tm->rotation, &tm->position, objects[i].type,
            SCREEN_SUBPIXEL_CENTER_X, SCREEN_SUBPIXEL_CENTER_Y);
    }
}

//

#define GAMEPLAY_INVALID_TARGET 0xff

#define GAMEPLAY_TARGETING_DURATION_BITS 5
#define GAMEPLAY_TARGETING_DURATION (1 << GAMEPLAY_TARGETING_DURATION_BITS)

//

#define DISPLAY_STATE_MAIN              0
#define DISPLAY_STATE_STATUS            1
#define DISPLAY_STATE_COMMUNICATIONS    2
#define DISPLAY_STATE_CARGO_MANIFEST    3
#define DISPLAY_STATE_PERIPHERALS       4
#define DISPLAY_STATE_REPAIR_DROID      5
#define DISPLAY_STATE_TRACTOR_BEAM      6

static const char* display_options_main[] = {
    "1. Status",
    "2. Communications",
    "3. Cargo manifest",
    "4. Peripherals",
    NULL
};

static const char* display_options_status[] = {
    "1. Back",
    NULL
};

static const char* display_options_communications[] = {
    "1. Back",
    NULL
};

static const char* display_options_cargo_manifest[] = {
    "1. Back",
    NULL
};

static const char* display_options_peripherals[] = {
    "1. Back",
    "2. Repair droid",
    "3. Tractor beam",
    NULL
};

static const char* display_options_repair_droid[] = {
    "1. Back",
    NULL
};

static const char* display_options_tractor_beam[] = {
    "1. Back",
    NULL
};

static const char* const text_main_menu = "Main menu";
static const char* const text_status = "Status";
static const char* const text_communications = "Communications";
static const char* const text_cargo_manifest = "Cargo manifest";
static const char* const text_peripherals = "Peripherals";
static const char* const text_repair_droid = "Repair droid";
static const char* const text_tractor_beam = "Tractor beam";
static const char* const text_not_connected = "Not connected";
static const char* const text_no_cargo = "No cargo";
static const char* const text_offline = "Offline";

//

static struct {
    uint8_t targeting_timer;
    uint8_t targeted_object;

    fx3_t forward;
    fx3_t up;

    fx_t yaw_speed;
    fx_t pitch_speed;

    fx3_t linear_velocity;

    fx_t energy;

    uint16_t set_speed;
    uint16_t actual_speed;

    uint8_t turning_left;
    uint8_t turning_right;
    uint8_t turning_up;
    uint8_t turning_down;

    uint8_t accelerating;
    uint8_t decelerating;
    uint8_t fullstop;
    uint8_t afterburner;

    uint8_t changing_target;
    uint8_t select;

    uint8_t display_state;

    uint32_t hidden_text_start_tick;
} gameplay = { 0 };

static void init_gameplay() {
    memset(&gameplay, 0, sizeof(gameplay));

    gameplay.forward.x = -FX_ONE;
    gameplay.up.z = FX_ONE;

    gameplay.energy = FX_ONE;
    gameplay.set_speed = 150;

    {
        fx3_t start_position;
        fx3_mul(&start_position, &gameplay.forward, -FX_ONE * 5);
        init_objects(&start_position);
    }
}

static inline int is_targeting() {
    return gameplay.targeted_object != GAMEPLAY_INVALID_TARGET &&
        objects[gameplay.targeted_object].type != OBJECT_TYPE_NONE;
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
        draw_ship(&view_matrix, &rotation, &translation,
            SCREEN_SUBPIXEL_CENTER_X, SCREEN_SUBPIXEL_CENTER_Y);
    }

    flush_mesh_draw_buffer(draw_mode);
}

#else

static inline int is_benchmark_running() { return 0; }
static inline void update_benchmark() {}
static inline void draw_benchmark() {}

#endif

//

static void draw_still_images() {
    fx4x3_t view_matrix;
    fx4_t rotation = { 0, 0, 0, FX_ONE };
    fx3_t translation = { 0, 0, 0 };
    fx3_t up = { 0, 0, FX_ONE };

    {
        fx3_t target = { 0, 200, 200 };
        fx3_t eye = { -16000, 16000, 6000 };
        fx4x3_look_at(&view_matrix, &eye, &target, &up);
        draw_object(&view_matrix, &rotation, &translation, OBJECT_TYPE_ABANDONED_SHIP,
            80 << RASTER_SUBPIXEL_BITS, 50 << RASTER_SUBPIXEL_BITS);
    }
    {
        fx3_t target = { 0, 0, 200 };
        fx3_t eye = { -6000, 6000, 3000 };
        fx4x3_look_at(&view_matrix, &eye, &target, &up);
        draw_object(&view_matrix, &rotation, &translation, OBJECT_TYPE_CONTAINER,
            (160 + 80) << RASTER_SUBPIXEL_BITS, 50 << RASTER_SUBPIXEL_BITS);
    }
    {
        fx3_t target = { 0, 200, 0 };
        fx3_t eye = { -14000, 14000, 10000 };
        fx4x3_look_at(&view_matrix, &eye, &target, &up);
        draw_object(&view_matrix, &rotation, &translation, OBJECT_TYPE_SHIP_WRECK,
            80 << RASTER_SUBPIXEL_BITS, (100 + 50) << RASTER_SUBPIXEL_BITS);
    }

    clear_memory(dblbuf, 0, SCREEN_NUM_PIXELS);
    flush_mesh_draw_buffer(0);
    copy_memory(still_images, dblbuf, SCREEN_NUM_PIXELS);
}

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

#define TURNING_ACCELERATION 400
#define TURNING_SPEED_DAMPENING 63200
#define MAX_TURNING_SPEED FX_ONE
#define TURNING_SPEED_LOW_LIMIT 64

#define LINEAR_ACCELERATION 1000

#define GAMEPLAY_SPEED_TO_WORLD_MUL (FX_ONE * 180)
#define GAMEPLAY_SPEED_FROM_WORLD_MUL (FX_ONE / 180)

#define MAX_GAMEPLAY_SPEED 300
#define GAMEPLAY_SPEED_STEP 2

#define GAMEPLAY_AFTERBURNER_SPEED 600
#define GAMEPLAY_AFTERBURNER_ENERGY_USAGE (FX_ONE / 200)

#define GAMEPLAY_ENERGY_REGEN (FX_ONE / 400)

static const fx4_t IDENTITY_QUAT = { 0, 0, 0, FX_ONE };

static uint16_t calc_object_distance(uint8_t index) {
    return fx3_length(&object_tms[index].position) >> 8;
}

static void update() {
    uint16_t i = 0;

    if (gameplay.changing_target) {
        gameplay.changing_target = 0;
        gameplay.targeting_timer = 0;

        gameplay.targeted_object++;
        if (gameplay.targeted_object == num_objects)
            gameplay.targeted_object = 0;

        sfx_processing();
    }

    switch (gameplay.display_state) {
        case DISPLAY_STATE_MAIN: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_STATUS;
                sfx_select();
            } else if (gameplay.select & 2) {
                gameplay.display_state = DISPLAY_STATE_COMMUNICATIONS;
                sfx_select();
            } else if (gameplay.select & 4) {
                gameplay.display_state = DISPLAY_STATE_CARGO_MANIFEST;
                sfx_select();
            } else if (gameplay.select & 8) {
                gameplay.display_state = DISPLAY_STATE_PERIPHERALS;
                sfx_select();
            }

            break;
        }

        case DISPLAY_STATE_STATUS: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_MAIN;
                sfx_back();
            }

            break;
        }

        case DISPLAY_STATE_COMMUNICATIONS: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_MAIN;
                sfx_back();
            }

            break;
        }

        case DISPLAY_STATE_CARGO_MANIFEST: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_MAIN;
                sfx_back();
            }

            break;
        }

        case DISPLAY_STATE_PERIPHERALS: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_MAIN;
                sfx_back();
            } else if (gameplay.select & 2) {
                gameplay.display_state = DISPLAY_STATE_REPAIR_DROID;
                sfx_select();
            } else if (gameplay.select & 4) {
                gameplay.display_state = DISPLAY_STATE_TRACTOR_BEAM;
                sfx_select();
            }

            break;
        }

        case DISPLAY_STATE_REPAIR_DROID: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_PERIPHERALS;
                sfx_back();
            }

            break;
        }

        case DISPLAY_STATE_TRACTOR_BEAM: {
            if (gameplay.select & 1) {
                gameplay.display_state = DISPLAY_STATE_PERIPHERALS;
                sfx_back();
            }

            break;
        }

        default:
            aw_assert(0);
            break;
    }

    gameplay.select = 0;

    for (i = 0; i < timing.ticks_to_advance; ++i) {
        uint16_t gameplay_target_speed;
        fx_t energy_cut = 0;

        {
            uint16_t previous_set_speed = gameplay.set_speed;

            if (gameplay.fullstop) {
                gameplay.fullstop = 0;
                gameplay.set_speed = 0;
            } else {
                if (gameplay.accelerating && gameplay.set_speed < MAX_GAMEPLAY_SPEED)
                    gameplay.set_speed += GAMEPLAY_SPEED_STEP;

                if (gameplay.decelerating && gameplay.set_speed != 0)
                    gameplay.set_speed -= GAMEPLAY_SPEED_STEP;
            }

            if (gameplay.set_speed != previous_set_speed &&
                gameplay.set_speed % 20 == 0) {
                sfx_short_scrub();
            }
        }

        if (gameplay.afterburner &&
            gameplay.energy >= GAMEPLAY_AFTERBURNER_ENERGY_USAGE) {
            static uint8_t afterburner_sfx_counter = 25;
            afterburner_sfx_counter--;
            if (afterburner_sfx_counter == 0) {
                afterburner_sfx_counter = 25;
                sfx_white_noise();
            }

            gameplay_target_speed = GAMEPLAY_AFTERBURNER_SPEED;
            energy_cut += GAMEPLAY_AFTERBURNER_ENERGY_USAGE;
        } else {
            gameplay_target_speed = gameplay.set_speed;
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

            fx3_mul_ip(&gameplay.linear_velocity, FX_ONE - (FX_ONE >> 7));

            if (gameplay_target_speed) {
                fx_t actual_speed = fx3_length(&gameplay.linear_velocity);
                fx_t target_speed = fx_mul(gameplay_target_speed,
                    GAMEPLAY_SPEED_TO_WORLD_MUL);
                if (actual_speed < target_speed) {
                    fx3_t acceleration;
                    fx_t mul = fx_min((LINEAR_ACCELERATION >> 3) +
                        target_speed - actual_speed, LINEAR_ACCELERATION);
                    fx3_mul(&acceleration, &gameplay.forward, mul);
                    fx3_add_ip(&gameplay.linear_velocity, &acceleration);
                }
            }

            gameplay.actual_speed = fx_mul(fx3_length(&gameplay.linear_velocity),
                GAMEPLAY_SPEED_FROM_WORLD_MUL);
        }

        gameplay.energy -= energy_cut;
        gameplay.energy += GAMEPLAY_ENERGY_REGEN;
        if (gameplay.energy > FX_ONE)
            gameplay.energy = FX_ONE;

        if (gameplay.targeted_object != GAMEPLAY_INVALID_TARGET &&
            objects[gameplay.targeted_object].type != OBJECT_TYPE_NONE) {
            if (gameplay.targeting_timer != GAMEPLAY_TARGETING_DURATION)
                gameplay.targeting_timer++;
        }

        {
            fx3_t camera_target, movement;

            camera_target.x = gameplay.forward.x >> 1;
            camera_target.y = gameplay.forward.y >> 1;
            camera_target.z = gameplay.forward.z >> 1;

            movement.x = -gameplay.linear_velocity.x >> 8;
            movement.y = -gameplay.linear_velocity.y >> 8;
            movement.z = -gameplay.linear_velocity.z >> 8;

            update_objects(&movement);
            update_debris(&camera_target, &movement);
        }
    }

    if (gameplay.hidden_text_start_tick == 0 && is_targeting() &&
        (objects[gameplay.targeted_object].type == OBJECT_TYPE_NAV_POINT ||
        objects[gameplay.targeted_object].type == OBJECT_TYPE_JUMP_POINT) &&
        calc_object_distance(gameplay.targeted_object) < 1000) {
        gameplay.hidden_text_start_tick = timing.current_tick;
    }
}

static void draw_fps() {
    int16_t text_width;
    char buf[8] = { 0 };
    uint16_t v = minu32(fps.average, 9999);

    if (vsync) {
        buf[0] = '*';
        buf[1] = ' ';
        number_to_string(buf + 2, v);
    } else {
        number_to_string(buf, v);
    }

    text_width = calc_text_width(buf);
    draw_text2(buf, 320 - 5 - text_width, 4, 8, 2);
}

#define CROSSHAIR_LOW_X 4
#define CROSSHAIR_HIGH_X 9
#define CROSSHAIR_LOW_Y 4
#define CROSSHAIR_HIGH_Y 8

static void draw_crosshair() {
    draw_hline_no_check(SCREEN_CENTER_X - CROSSHAIR_HIGH_X, SCREEN_CENTER_X - CROSSHAIR_LOW_X, SCREEN_CENTER_Y, 91);
    draw_hline_no_check(SCREEN_CENTER_X + CROSSHAIR_LOW_X, SCREEN_CENTER_X + CROSSHAIR_HIGH_X, SCREEN_CENTER_Y, 91);
    draw_vline_no_check(SCREEN_CENTER_X, SCREEN_CENTER_Y - CROSSHAIR_HIGH_Y, SCREEN_CENTER_Y - CROSSHAIR_LOW_Y, 91);
    draw_vline_no_check(SCREEN_CENTER_X, SCREEN_CENTER_Y + CROSSHAIR_LOW_Y, SCREEN_CENTER_Y + CROSSHAIR_HIGH_Y, 91);
}

#define TARGETING_BOX_HALF_WIDTH 12
#define TARGETING_BOX_HALF_HEIGHT 10

static void draw_targeting(const fx4x3_t* view_matrix,
    const fx3_t* target_position, uint8_t object_type, uint8_t timer) {
    uint8_t w, h, scale;

    fx3_t p;
    fx_transform_point(&p, view_matrix, target_position);
    if (p.z < NEAR_CLIP)
        return;

    project_to_screen(&p, SCREEN_SUBPIXEL_CENTER_X, SCREEN_SUBPIXEL_CENTER_Y);
    projected_position_to_screen(&p);

    scale = (GAMEPLAY_TARGETING_DURATION * 3) - (timer << 1);
    w = (TARGETING_BOX_HALF_WIDTH * scale) >> GAMEPLAY_TARGETING_DURATION_BITS;
    h = (TARGETING_BOX_HALF_HEIGHT * scale) >> GAMEPLAY_TARGETING_DURATION_BITS;

    draw_box_outline(p.x - w, p.y - h, p.x + w, p.y + h, 6);

    if (object_type == OBJECT_TYPE_NAV_POINT ||
        object_type == OBJECT_TYPE_JUMP_POINT) {
        draw_hline(p.x - 2, p.x + 2, p.y, 2);
        draw_vline(p.x, p.y - 2, p.y + 2, 2);
    }
}

#define HELP_TEXT_COLOR 4
#define HELP_TEXT_X 10
#define HELP_TEXT_Y 20
#define HELP_TEXT_LINE_SPACING 7
#define HELP_TEXT_PARAGRAPH_SPACING 12

static void draw_help() {
    int16_t y = HELP_TEXT_Y;
    draw_text2("Build 2023-10-15", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_PARAGRAPH_SPACING;

    draw_text("[Controls]", HELP_TEXT_X, y, HELP_TEXT_COLOR); y += HELP_TEXT_LINE_SPACING;
    draw_text2("Arrow keys: Maneuver", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("+/-: Speed adjustment", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("TAB: Afterburner", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("0: Full stop", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("T: Next target", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("1-9: Display selection", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_PARAGRAPH_SPACING;

    draw_text("[Useful keys]", HELP_TEXT_X, y, HELP_TEXT_COLOR); y += HELP_TEXT_LINE_SPACING;
    draw_text2("H/F1: Help", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("V: Toggle vertical sync", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("ESC: Exit to DOS", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_PARAGRAPH_SPACING;

    draw_text("Made for DOS COM Jam 2023", HELP_TEXT_X, y, HELP_TEXT_COLOR); y += HELP_TEXT_LINE_SPACING;
    draw_text2("https://aarnig.itch.io/dos-com-jam", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
    draw_text2("aarni.gratseff@gmail.com", HELP_TEXT_X, y, HELP_TEXT_COLOR, 2); y += HELP_TEXT_LINE_SPACING;
}

static const char* const TEXT = "Thank you for your interest in this playable demo!";

static void draw_texts() {
    char buffer[64];
    int16_t x, y, text_width;
    uint8_t i;
    uint32_t t;

    if (gameplay.hidden_text_start_tick == 0)
        return;

    t = (timing.current_tick - gameplay.hidden_text_start_tick) >> 4;

    if (t > 256)
        return;

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

    text_width = calc_text_width(buffer);

    x = (SCREEN_WIDTH - text_width) >> 1;
    y = SCREEN_HEIGHT >> 2;

    x = draw_text(buffer, x, y, 4);

    if (((t >> 1) & 1) == 0)
        draw_text_cursor(x, y, 7);
}

#if 0
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
#endif

static void blit_image(int16_t x, int16_t y, int16_t src_x, int16_t src_y,
    int16_t width, int16_t height, uint8_t __far* src) {
    int16_t i, j;
    uint8_t __far* tgt = dblbuf + (mul_by_screen_stride(y) + x);
    src += mul_by_screen_stride(src_y) + src_x;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            uint8_t c = *src++;
            if (c)
                *tgt = c;
            tgt++;
        }

        tgt += SCREEN_STRIDE - width;
        src += SCREEN_STRIDE - width;
    }
}

#define LINE_SPACING 8

static void draw_centered_text(const char* text, int16_t x, int16_t y, uint8_t c) {
    int16_t text_width = calc_text_width(text);
    draw_text2(text, x - (text_width >> 1), y, c, 2);
}

static void draw_distance_text(int16_t x, int16_t y, fx_t distance) {
    static const char* DISTANCE_TEXT = "Distance: ";
    char buf[32] = { 0 };
    int16_t text_width;
    memcpy(buf, DISTANCE_TEXT, 10);

    if (distance < 32768) {
        uint16_t v = minu32(distance, 65535L);
        number_to_string(buf + 10, v);
    } else {
        buf[10] = 'f';
        buf[11] = 'a';
        buf[12] = 'r';
    }

    text_width = calc_text_width(buf);
    draw_text2(buf, x - (text_width >> 1), y + LINE_SPACING, 4, 2);
}

static inline void put_pixel(int16_t x, int16_t y, uint8_t c) {
    if (x < 0 || y < 0 || x > SCREEN_X_MAX || y > SCREEN_Y_MAX)
        return;

    dblbuf[mul_by_screen_stride(y) + x] = c;
}

static void draw_display_options(int16_t x, int16_t y, const char* const* texts) {
    while (*texts) {
        draw_text2(*texts, x, y, 4, 2);
        y += LINE_SPACING;
        texts++;
    }
}

#define RADAR_X (SCREEN_WIDTH / 2)
#define RADAR_Y (SCREEN_HEIGHT - 38)

#define RADAR_HALF_WIDTH 30
#define RADAR_HALF_HEIGHT (RADAR_HALF_WIDTH * 5 / 6)

#define RADAR_INNER_HALF_WIDTH 12
#define RADAR_INNER_HALF_HEIGHT 10

#define RADAR_COLOR_0 124
#define RADAR_COLOR_1 122
#define RADAR_CROSS_COLOR 115

static void draw_gameplay_ui(const fx4x3_t* view_matrix) {
    if (is_targeting())
        draw_targeting(view_matrix,
            &object_tms[gameplay.targeted_object].position,
            objects[gameplay.targeted_object].type,
            gameplay.targeting_timer);

    draw_crosshair();

    {
        int16_t x = 8;
        int16_t y = 122;
        int16_t width = 104;
        int16_t height = 72;

        draw_darkened_box(x, y, x + width, y + height);

        {
            int16_t tx = x + 5;
            int16_t ty = y + 5 + LINE_SPACING;
            const char* caption = NULL;

            switch (gameplay.display_state) {
                case DISPLAY_STATE_MAIN:
                    caption = text_main_menu;
                    draw_display_options(tx, ty, display_options_main);
                    break;

                case DISPLAY_STATE_STATUS:
                    caption = text_status;
                    draw_display_options(tx, ty, display_options_status);
                    break;

                case DISPLAY_STATE_COMMUNICATIONS:
                    caption = text_communications;
                    draw_display_options(tx, ty, display_options_communications);
                    if ((timing.current_tick >> 5) & 1)
                        draw_centered_text(text_offline, x + (width >> 1), y + (height >> 1) - 2, 12);
                    break;

                case DISPLAY_STATE_CARGO_MANIFEST:
                    caption = text_cargo_manifest;
                    draw_display_options(tx, ty, display_options_cargo_manifest);
                    draw_centered_text(text_no_cargo, x + (width >> 1), y + (height >> 1) - 2, 4);
                    break;

                case DISPLAY_STATE_PERIPHERALS:
                    caption = text_peripherals;
                    draw_display_options(tx, ty, display_options_peripherals);
                    break;

                case DISPLAY_STATE_REPAIR_DROID:
                    caption = text_tractor_beam;
                    draw_display_options(tx, ty, display_options_repair_droid);
                    if ((timing.current_tick >> 5) & 1)
                        draw_centered_text(text_not_connected, x + (width >> 1), y + (height >> 1) - 2, 12);
                    break;

                case DISPLAY_STATE_TRACTOR_BEAM:
                    caption = text_tractor_beam;
                    draw_display_options(tx, ty, display_options_tractor_beam);
                    if ((timing.current_tick >> 5) & 1)
                        draw_centered_text(text_not_connected, x + (width >> 1), y + (height >> 1) - 2, 12);
                    break;

                default:
                    aw_assert(0);
                    break;
            }

            if (caption)
                draw_centered_text(caption, x + (width >> 1), y + 5, 4);
        }

        x = SCREEN_WIDTH - 8 - width;
        draw_darkened_box(x, y, x + width, y + height);

        if (is_targeting()) {
            const char* object_name = NULL;

            switch (objects[gameplay.targeted_object].type) {
                case OBJECT_TYPE_SHIP_WRECK:
                    blit_image(x, y, 28, 100 + 18, width, height, still_images);
                    object_name = "Ship wreck";
                    break;

                case OBJECT_TYPE_ABANDONED_SHIP:
                    blit_image(x, y, 28, 18, width, height, still_images);
                    object_name = "Abandoned ship";
                    break;

                case OBJECT_TYPE_CONTAINER:
                    blit_image(x, y, 160 + 28, 18, width, height, still_images);
                    object_name = "Container";
                    break;

                case OBJECT_TYPE_NAV_POINT:
                    object_name = "Nav point";
                    break;

                case OBJECT_TYPE_JUMP_POINT:
                    object_name = "Jump point";
                    break;
            }

            if (object_name) draw_centered_text(object_name, x + (width >> 1), y + 5, 4);
            draw_distance_text(x + (width >> 1), y + height - 12 - 5,
                calc_object_distance(gameplay.targeted_object));
        } else {
            draw_centered_text("No target", x + (width >> 1), y + 5, 4);
        }
    }

    {
        int16_t left = SCREEN_WIDTH / 2 - 42;
        int16_t right = SCREEN_WIDTH / 2 + 30;
        int16_t y = 122 + 5;
        char buf[5] = { 0 };
        int16_t text_width;

        draw_text2("SET", left, y, 4, 2);
        draw_text2("ACT", right, y, 4, 2);

        y += LINE_SPACING;

        number_to_string(buf, gameplay.afterburner ? GAMEPLAY_AFTERBURNER_SPEED : gameplay.set_speed);
        text_width = calc_text_width(buf);
        draw_text2(buf, left + 7 - (text_width >> 1), y, 4, 2);

        number_to_string(buf, gameplay.actual_speed);
        text_width = calc_text_width(buf);
        draw_text2(buf, right + 7 - (text_width >> 1), y, 4, 2);
    }

    {
        uint8_t i;
        uint8_t energy_bars = gameplay.energy >> 12;
        uint16_t x = SCREEN_WIDTH / 2 - 31;
        uint16_t y = 5;

        draw_text2("E", x - 7, y + 1, 8, 2);

        for (i = 0; i < energy_bars; ++i) {
            draw_box_outline(x, y, x + 2, y + 6, 11);
            x += 4;
        }

        for (i = energy_bars; i < 16; ++i) {
            draw_box_outline(x, y, x + 2, y + 6, 0);
            x += 4;
        }
    }

    draw_hline_no_check(RADAR_X - RADAR_HALF_WIDTH,
        RADAR_X - RADAR_INNER_HALF_WIDTH, RADAR_Y, RADAR_CROSS_COLOR);
    draw_hline_no_check(RADAR_X + RADAR_INNER_HALF_WIDTH,
        RADAR_X + RADAR_HALF_WIDTH, RADAR_Y, RADAR_CROSS_COLOR);

    draw_vline_no_check(RADAR_X, RADAR_Y - RADAR_HALF_HEIGHT,
        RADAR_Y - RADAR_INNER_HALF_HEIGHT, RADAR_CROSS_COLOR);
    draw_vline_no_check(RADAR_X, RADAR_Y + RADAR_INNER_HALF_HEIGHT,
        RADAR_Y + RADAR_HALF_HEIGHT, RADAR_CROSS_COLOR);

    {
        uint8_t i;
        int16_t x, y;
        fx3_t dir;
        for (i = 0; i < num_objects; ++i) {
            fx_transform_point(&dir, view_matrix, &object_tms[i].position);
            if (dir.z < 0)
                dir.z = 0;

            fx3_normalize_ip(&dir);

            x = RADAR_X + fx_mul(dir.x, RADAR_HALF_WIDTH);
            y = RADAR_Y - fx_mul(dir.y, RADAR_HALF_HEIGHT);

            if (gameplay.targeted_object == i) {
                put_pixel(x, y - 1, RADAR_COLOR_0);
                put_pixel(x - 1, y, RADAR_COLOR_0);
                put_pixel(x, y, RADAR_COLOR_1);
                put_pixel(x + 1, y, RADAR_COLOR_0);
                put_pixel(x, y + 1, RADAR_COLOR_0);
            } else {
                put_pixel(x, y, RADAR_COLOR_0);
            }
        }
    }

    {
#define METER_X0 (SCREEN_WIDTH / 2 - 40)
#define METER_X1 (METER_X0 + 8)
#define METER_Y (SCREEN_HEIGHT - 17)
#define METER_C 122
        draw_hline_no_check(METER_X0, METER_X1, METER_Y, METER_C);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 2, METER_C + 1);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 4, METER_C + 2);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 6, METER_C + 3);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 8, METER_C + 4);
#undef METER_X0
#define METER_X0 (SCREEN_WIDTH / 2 + 40 - 8)
        draw_hline_no_check(METER_X0, METER_X1, METER_Y, METER_C + 4);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 2, METER_C + 3);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 4, METER_C + 2);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 6, METER_C + 1);
        draw_hline_no_check(METER_X0, METER_X1, METER_Y + 8, METER_C);
#undef METER_X0
#undef METER_X1
#undef METER_Y
#undef METER_C
    }

    draw_texts();
}

static void draw() {
    fx4x3_t view_matrix;
    fx4x3_look_at(&view_matrix, NULL, &gameplay.forward, &gameplay.up);

    if (stars_enabled)
        draw_stars(&view_matrix);

    if (debris_enabled)
        draw_debris(&view_matrix);

    draw_objects(&view_matrix);

    flush_mesh_draw_buffer(draw_mode);

    if (help) {
        draw_help();
    } else {
        draw_gameplay_ui(&view_matrix);
    }

    if (fps_enabled) {
#if defined(BENCHMARK_ENABLED)
        if (benchmark_result) {
            char buf[6] = { 0 };
            number_to_string(buf, minu32(benchmark_result, 65535L));
            draw_text(buf, 320 - 48, 4, 8);
        }
#endif

        draw_fps();
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

            case KEY_0: gameplay.fullstop = 1; break;

            case KEY_TAB: gameplay.afterburner = 1; break;
            case KEY_TAB | KEY_UP_FLAG: gameplay.afterburner = 0; break;

            case KEY_T: gameplay.changing_target = 1; break;

            case KEY_ESC:
                quit = 1;
                break;

            case KEY_1: gameplay.select |= 1; break;
            case KEY_2: gameplay.select |= 2; break;
            case KEY_3: gameplay.select |= 4; break;
            case KEY_4: gameplay.select |= 8; break;

            case KEY_V:
                vsync ^= 1;
                break;

#if defined(BENCHMARK_ENABLED)
            case KEY_B:
                start_benchmark();
                break;
#endif

            case KEY_H:
            case KEY_F1:
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

    dblbuf = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!dblbuf)
        goto exit;

    still_images = (uint8_t __far*)_fmalloc(SCREEN_NUM_PIXELS);
    if (!still_images)
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

    //

    opl_init();
    opl_warmup();

    //

    draw_still_images();

    //

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

        clear_memory(dblbuf, 0x08, SCREEN_NUM_PIXELS);

        if (!is_benchmark_running()) {
            draw();
            if (vsync)
                vga_wait_for_retrace();
        } else {
            draw_benchmark();
        }

        vga_copy(dblbuf);
    }

exit:
    _ffree(sort_buffer);
    _ffree(draw_buffer);
    _ffree(tm_buffer);
    _ffree(still_images);
    _ffree(dblbuf);

    vga_set_mode(0x3);

    if (keyboard_initialized) keyb_cleanup();
    if (timer_initialized) timer_cleanup();

    opl_done();

    putz(exit_message);
    set_text_cursor(1, 0);
    kb_clear_buffer();
}
