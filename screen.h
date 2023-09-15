#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define SCREEN_CENTER_X (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT / 2)

#define SCREEN_STRIDE SCREEN_WIDTH

static inline uint16_t mul_by_screen_stride(uint16_t x) {
    return (x << 8) + (x << 6);
}

static inline int32_t mul_by_screen_stride32(int32_t x) {
    return (x << 8) + (x << 6);
}

#endif
