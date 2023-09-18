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

#endif
