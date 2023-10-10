#define TIMER_TICK_USEC 858

volatile uint32_t timer_ticks;

static inline uint32_t read_timer_ticks() {
    uint32_t r;
    _disable();
    r = timer_ticks;
    _enable();
    return r;
}

// defined in timer.asm
void timer_init();
void timer_cleanup();
