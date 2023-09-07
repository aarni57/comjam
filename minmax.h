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

static inline uint16_t minu16(uint16_t a, uint16_t b) {
    return a < b ? a : b;
}

static inline uint16_t maxu16(uint16_t a, uint16_t b) {
    return a > b ? a : b;
}

static inline uint16_t clampu16(uint16_t x, uint16_t l, uint16_t h) {
    return minu16(maxu16(x, l), h);
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

//

static inline uint32_t minu32(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

static inline uint32_t maxu32(uint32_t a, uint32_t b) {
    return a > b ? a : b;
}

static inline uint32_t clampu32(uint32_t x, uint32_t l, uint32_t h) {
    return minu32(maxu32(x, l), h);
}

#endif
