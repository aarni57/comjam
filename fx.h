#ifndef FX_H
#define FX_H

#include "fxtypes.h"

//

#define FX_DECIMAL_BITS 16
#define FX_ONE (1L << FX_DECIMAL_BITS)
#define FX_DECIMAL_MASK (FX_ONE - 1)
#define FX_HALF (FX_ONE / 2)

#define fx_min min32
#define fx_max max32
#define fx_clamp clamp32

#define fx_abs abs

//

static inline fx_t fx_mul(fx_t x, fx_t y) {
    int32_t a, c;
    uint32_t b, d;

    a = x >> 16;
    b = x & 0xffff;

    c = y >> 16;
    d = y & 0xffff;

    return ((a * c) << 16) + a * d + c * b + ((b * d) >> 16);
}

//

#define SIN_TABLE_SIZE 0x80
#define SIN_TABLE_SIZE_MASK 0x7f
#define SIN_TABLE_SIZE_SHIFT 7
#define SIN_TABLE_FX_FRACTION_BITS 7
#define SIN_TABLE_FX_FRACTION_MASK 0x7f

static const uint16_t SIN_TABLE[] = {
    0, 804, 1608, 2412, 3216, 4019, 4821, 5623,
    6424, 7224, 8022, 8820, 9616, 10411, 11204, 11996,
    12785, 13573, 14359, 15143, 15924, 16703, 17479, 18253,
    19024, 19792, 20557, 21320, 22078, 22834, 23586, 24335,
    25080, 25821, 26558, 27291, 28020, 28745, 29466, 30182,
    30893, 31600, 32303, 33000, 33692, 34380, 35062, 35738,
    36410, 37076, 37736, 38391, 39040, 39683, 40320, 40951,
    41576, 42194, 42806, 43412, 44011, 44604, 45190, 45769,
    46341, 46906, 47464, 48015, 48559, 49095, 49624, 50146,
    50660, 51166, 51665, 52156, 52639, 53114, 53581, 54040,
    54491, 54934, 55368, 55794, 56212, 56621, 57022, 57414,
    57798, 58172, 58538, 58896, 59244, 59583, 59914, 60235,
    60547, 60851, 61145, 61429, 61705, 61971, 62228, 62476,
    62714, 62943, 63162, 63372, 63572, 63763, 63944, 64115,
    64277, 64429, 64571, 64704, 64827, 64940, 65043, 65137,
    65220, 65294, 65358, 65413, 65457, 65492, 65516, 65531,
    // This table is actually 2 * table size
    // The second quadrant (descending):
    65535, 65531, 65516, 65492, 65457, 65413, 65358, 65294,
    65220, 65137, 65043, 64940, 64827, 64704, 64571, 64429,
    64277, 64115, 63944, 63763, 63572, 63372, 63162, 62943,
    62714, 62476, 62228, 61971, 61705, 61429, 61145, 60851,
    60547, 60235, 59914, 59583, 59244, 58896, 58538, 58172,
    57798, 57414, 57022, 56621, 56212, 55794, 55368, 54934,
    54491, 54040, 53581, 53114, 52639, 52156, 51665, 51166,
    50660, 50146, 49624, 49095, 48559, 48015, 47464, 46906,
    46341, 45769, 45190, 44604, 44011, 43412, 42806, 42194,
    41576, 40951, 40320, 39683, 39040, 38391, 37736, 37076,
    36410, 35738, 35062, 34380, 33692, 33000, 32303, 31600,
    30893, 30182, 29466, 28745, 28020, 27291, 26558, 25821,
    25080, 24335, 23586, 22834, 22078, 21320, 20557, 19792,
    19024, 18253, 17479, 16703, 15924, 15143, 14359, 13573,
    12785, 11996, 11204, 10411, 9616, 8820, 8022, 7224,
    6424, 5623, 4821, 4019, 3216, 2412, 1608, 804
};

static inline fx_t lookup_sin_table(uint16_t index) {
    const uint16_t quadrant = (index >> SIN_TABLE_SIZE_SHIFT) & 3;
    const uint16_t index_in_quadrant = index & SIN_TABLE_SIZE_MASK;
    switch (quadrant) {
        case 0: return SIN_TABLE[index_in_quadrant];
        case 1: return SIN_TABLE[index_in_quadrant + SIN_TABLE_SIZE];
        case 2: return -(fx_t)SIN_TABLE[index_in_quadrant];
        case 3: return -(fx_t)SIN_TABLE[index_in_quadrant + SIN_TABLE_SIZE];
        default: return 0;
    }
}

static inline fx_t fx_sin(fx_t x) {
    uint16_t d, i, t;
    fx_t y0, y1;

    if (x < 0) {
        x = -x + FX_HALF;
    }

    d = x & FX_DECIMAL_MASK;
    i = d >> (FX_DECIMAL_BITS - (SIN_TABLE_SIZE_SHIFT + 2));

    y0 = lookup_sin_table(i);
    y1 = lookup_sin_table(i + 1);

    t = (d - (i << (SIN_TABLE_SIZE_SHIFT + 2))) & SIN_TABLE_FX_FRACTION_MASK;
    return clamp32(y0 + (((y1 - y0) * t) >> SIN_TABLE_FX_FRACTION_BITS), -FX_ONE, FX_ONE);
}

static inline fx_t fx_cos(fx_t x) {
    return fx_sin(x + FX_ONE / 4);
}

//

static inline fx2_t fx2_rotate(fx2_t v, fx_t c, fx_t s) {
    fx2_t r;
    r.x = fx_mul(v.x, c) - fx_mul(v.y, s);
    r.y = fx_mul(v.y, c) + fx_mul(v.x, s);
    return r;
}

//

#define FX4X3_00 0
#define FX4X3_01 1
#define FX4X3_02 2
#define FX4X3_10 3
#define FX4X3_11 4
#define FX4X3_12 5
#define FX4X3_20 6
#define FX4X3_21 7
#define FX4X3_22 8
#define FX4X3_30 9
#define FX4X3_31 10
#define FX4X3_32 11

static inline void fx3x3_identity(fx3x3_t* m) {
    m->m[0] = FX_ONE;
    m->m[1] = 0;
    m->m[2] = 0;
    m->m[3] = 0;
    m->m[4] = FX_ONE;
    m->m[5] = 0;
    m->m[6] = 0;
    m->m[7] = 0;
    m->m[8] = FX_ONE;
}

static inline void fx4x3_identity(fx4x3_t* m) {
    m->m[0] = FX_ONE;
    m->m[1] = 0;
    m->m[2] = 0;
    m->m[3] = 0;
    m->m[4] = FX_ONE;
    m->m[5] = 0;
    m->m[6] = 0;
    m->m[7] = 0;
    m->m[8] = FX_ONE;
    m->m[9] = 0;
    m->m[10] = 0;
    m->m[11] = 0;
}

static inline void fx4x3_translation(fx4x3_t* m, fx_t x, fx_t y, fx_t z) {
    m->m[0] = FX_ONE;
    m->m[1] = 0;
    m->m[2] = 0;
    m->m[3] = 0;
    m->m[4] = FX_ONE;
    m->m[5] = 0;
    m->m[6] = 0;
    m->m[7] = 0;
    m->m[8] = FX_ONE;
    m->m[9] = x;
    m->m[10] = y;
    m->m[11] = z;
}

static inline void fx_transform_point(fx3_t* r, const fx4x3_t* m, const fx3_t* v) {
    r->x = fx_mul(v->x, m->m[FX4X3_00]) + fx_mul(v->y, m->m[FX4X3_01]) + fx_mul(v->z, m->m[FX4X3_02]) + m->m[FX4X3_30];
    r->y = fx_mul(v->x, m->m[FX4X3_10]) + fx_mul(v->y, m->m[FX4X3_11]) + fx_mul(v->z, m->m[FX4X3_12]) + m->m[FX4X3_31];
    r->z = fx_mul(v->x, m->m[FX4X3_20]) + fx_mul(v->y, m->m[FX4X3_21]) + fx_mul(v->z, m->m[FX4X3_22]) + m->m[FX4X3_32];
}

static inline void fx4x3_mul(fx4x3_t* r, const fx4x3_t* a, const fx4x3_t* b) {
    r->m[FX4X3_00] = fx_mul(a->m[FX4X3_00], b->m[FX4X3_00]) +
        fx_mul(a->m[FX4X3_01], b->m[FX4X3_10]) +
        fx_mul(a->m[FX4X3_02], b->m[FX4X3_20]);
    r->m[FX4X3_01] = fx_mul(a->m[FX4X3_00], b->m[FX4X3_01]) +
        fx_mul(a->m[FX4X3_01], b->m[FX4X3_11]) +
        fx_mul(a->m[FX4X3_02], b->m[FX4X3_21]);
    r->m[FX4X3_02] = fx_mul(a->m[FX4X3_00], b->m[FX4X3_02]) +
        fx_mul(a->m[FX4X3_01], b->m[FX4X3_12]) +
        fx_mul(a->m[FX4X3_02], b->m[FX4X3_22]);
    r->m[FX4X3_10] = fx_mul(a->m[FX4X3_10], b->m[FX4X3_00]) +
        fx_mul(a->m[FX4X3_11], b->m[FX4X3_10]) +
        fx_mul(a->m[FX4X3_12], b->m[FX4X3_20]);
    r->m[FX4X3_11] = fx_mul(a->m[FX4X3_10], b->m[FX4X3_01]) +
        fx_mul(a->m[FX4X3_11], b->m[FX4X3_11]) +
        fx_mul(a->m[FX4X3_12], b->m[FX4X3_21]);
    r->m[FX4X3_12] = fx_mul(a->m[FX4X3_10], b->m[FX4X3_02]) +
        fx_mul(a->m[FX4X3_11], b->m[FX4X3_12]) +
        fx_mul(a->m[FX4X3_12], b->m[FX4X3_22]);
    r->m[FX4X3_20] = fx_mul(a->m[FX4X3_20], b->m[FX4X3_00]) +
        fx_mul(a->m[FX4X3_21], b->m[FX4X3_10]) +
        fx_mul(a->m[FX4X3_22], b->m[FX4X3_20]);
    r->m[FX4X3_21] = fx_mul(a->m[FX4X3_20], b->m[FX4X3_01]) +
        fx_mul(a->m[FX4X3_21], b->m[FX4X3_11]) +
        fx_mul(a->m[FX4X3_22], b->m[FX4X3_21]);
    r->m[FX4X3_22] = fx_mul(a->m[FX4X3_20], b->m[FX4X3_02]) +
        fx_mul(a->m[FX4X3_21], b->m[FX4X3_12]) +
        fx_mul(a->m[FX4X3_22], b->m[FX4X3_22]);
    fx_transform_point((fx3_t*)&r->m[FX4X3_30], a, (const fx3_t*)&b->m[FX4X3_30]);
}

#endif
