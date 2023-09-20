#ifndef FXTYPES_H
#define FXTYPES_H

#include <stdint.h>

typedef int32_t fx_t;

typedef struct fx2_t {
    fx_t x, y;
} fx2_t;

typedef struct fx3_t {
    fx_t x, y, z;
} fx3_t;

typedef struct fx3x3_t {
    fx_t m[9];
} fx3x3_t;

typedef struct fx4x3_t {
    fx_t m[12];
} fx4x3_t;

#endif
