#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define SCREEN_STRIDE SCREEN_WIDTH

static inline uint16_t mul_by_screen_stride(uint16_t x) {
    return (x << 8) + (x << 6);
}

#endif
