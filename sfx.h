static volatile uint16_t sfx_volume = 256;

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

#define NUM_SFX_EVENT_TYPES 11

static const sfx_event_type_t sfx_event_types[NUM_SFX_EVENT_TYPES] = {
    { 50000, 0, 0, 0 },
    { 40000, 80, 69, 28 },
    { 80000, 80, 57, 40 },
    { 200000, 80, 50, 48 },
    { 40000, 80, 62, 32 },
    { 50000, 80, 57, 32 },
    { 40000, 80, 62, 20 },
    { 20000, 80, 62, 28 },
    { 30000, 81, 57, 24 },
    { 200000, 121, 57, 40 },
    { 80000, 80, 50, 24 },
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

static inline void sfx_short_blip() {
    _disable();
    push_sfx_event(8);
    _enable();
}

static inline void sfx_blip() {
    _disable();
    push_sfx_event(3);
    _enable();
}

static inline void sfx_select() {
    _disable();
    push_sfx_event(5);
    push_sfx_event(2);
    _enable();
}

static inline void sfx_back() {
    _disable();
    push_sfx_event(5);
    push_sfx_event(6);
    _enable();
}

static inline void sfx_processing() {
    _disable();
    push_sfx_event(5);
    push_sfx_event(1);
    push_sfx_event(5);
    push_sfx_event(1);
    push_sfx_event(7);
    _enable();
}

static inline void sfx_error() {
    _disable();
    push_sfx_event(2);
    push_sfx_event(4);
    _enable();
}

static inline void sfx_short_error() {
    _disable();
    push_sfx_event(11);
    _enable();
}

static inline void sfx_short_scrub() {
    _disable();
    push_sfx_event(9);
    _enable();
}

static inline void sfx_white_noise() {
    _disable();
    push_sfx_event(10);
    _enable();
}
