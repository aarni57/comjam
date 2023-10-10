#include "song.h"

static volatile uint16_t music_volume = 256;

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
