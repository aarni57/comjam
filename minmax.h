#ifndef MINMAX_H
#define MINMAX_H

#include <stdint.h>

static inline uint8_t minu8(uint8_t a, uint8_t b) {
    return a < b ? a : b;
}

static inline uint8_t maxu8(uint8_t a, uint8_t b) {
    return a > b ? a : b;
}

static inline uint8_t clampu8(uint8_t x, uint8_t l, uint8_t h) {
    return minu8(maxu8(x, l), h);
}

//

static inline int16_t min16(int16_t a, int16_t b) {
    return a < b ? a : b;
}

static inline int16_t max16(int16_t a, int16_t b) {
    return a > b ? a : b;
}

static inline int16_t clamp16(int16_t x, int16_t l, int16_t h) {
    return min16(max16(x, l), h);
}

//

static inline int32_t min32(int32_t a, int32_t b) {
    return a < b ? a : b;
}

static inline int32_t max32(int32_t a, int32_t b) {
    return a > b ? a : b;
}

static inline int32_t clamp32(int32_t x, int32_t l, int32_t h) {
    return min32(max32(x, l), h);
}

#endif
