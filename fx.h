#ifndef FX_H
#define FX_H

#include "fxtypes.h"
#include "math386.h"

//

#define FX_DECIMAL_BITS 16L
#define FX_ONE 65536L
#define FX_DECIMAL_MASK 65535L
#define FX_HALF 32768L
#define FX_QUARTER 16384L

#define FX_EPSILON 256
#define FX_EPSILON_SQUARED 16

#define FX_MIN INT32_MIN
#define FX_MAX INT32_MAX

#define fx_min min32
#define fx_max max32
#define fx_clamp clamp32

static inline fx_t fx_abs(fx_t x) {
#if !defined(INLINE_ASM)
    return x < 0 ? -x : x;
#else
    __asm {
        .386
        mov eax, x
        cdq
        xor eax, edx
        sub eax, edx
        mov x, eax
    }

    return x;
#endif
}

#if !defined(INLINE_ASM)

static inline fx_t fx_mul(fx_t x, fx_t y) {
    int32_t a, c;
    uint32_t b, d;

    a = x >> 16;
    b = x & 0xffff;

    c = y >> 16;
    d = y & 0xffff;

    return ((a * c) << 16) + a * d + b * c + ((b * d) >> 16);
}

static inline fx_t fx_mul_ptr(const fx_t* x, const fx_t* y) {
    return fx_mul(*x, *y);
}

#elif 0

static inline fx_t fx_mul(fx_t x, fx_t y) {
    __asm {
        .386
        mov eax, x
        mov edx, y
        imul edx
        shrd eax, edx, 16
        mov x, eax
    }

    return x;
}

#else
fx_t fx_mul(fx_t x, fx_t y);
fx_t fx_mul_ptr(const fx_t* x, const fx_t* y);
#endif

#if !defined(INLINE_ASM)
#   define fx_pow2(x) fx_mul(x, x)
#elif 0
static inline fx_t fx_pow2(fx_t x) {
    __asm {
        .386
        mov eax, x
        mov edx, eax
        imul edx
        shrd eax, edx, 16
        mov x, eax
    }

    return x;
}
#else
fx_t fx_pow2(fx_t x);
#endif

#if !defined(INLINE_ASM)
static inline fx_t fx_lerp(fx_t a, fx_t b, fx_t t) {
    return a + fx_mul(b - a, t);
}
#else
static inline fx_t fx_lerp(fx_t x, fx_t y, fx_t t) {
    __asm {
        .386
        mov ebx, x
        mov eax, y
        sub eax, ebx

        mov ecx, t
        imul ecx
        shrd eax, edx, 16

        add eax, ebx
        mov x, eax
    }

    return x;
}
#endif

static inline fx_t fx_div(fx_t x, fx_t y) {
#if !defined(INLINE_ASM)
    uint32_t remainder, divider, quotient, bit;
    fx_t result;

    // This uses the basic binary restoring division algorithm.
    // It appears to be faster to do the whole division manually than
    // trying to compose a 64-bit divide out of 32-bit divisions on
    // platforms without hardware divide.

    if (y == 0)
        return 0;

    remainder = fx_abs(x);
    divider = fx_abs(y);

    quotient = 0;
    bit = 0x10000UL;

    /* The algorithm requires D >= R */
    while (divider < remainder) {
        divider <<= 1;
        bit <<= 1;
    }

    if (divider & 0x80000000UL) {
        // Perform one step manually to avoid overflows later.
        // We know that divider's bottom bit is 0 here.
        if (remainder >= divider) {
            quotient |= bit;
            remainder -= divider;
        }

        divider >>= 1;
        bit >>= 1;
    }

    /* Main division loop */
    while (bit && remainder) {
        if (remainder >= divider) {
            quotient |= bit;
            remainder -= divider;
        }

        remainder <<= 1;
        bit >>= 1;
    }

    result = quotient;

    /* Figure out the sign of result */
    if ((x ^ y) & 0x80000000UL) {
        result = -result;
    }

    return result;
#else
    __asm {
        .386
        mov edx, x
        mov ax, dx
        sar edx, 16
        sal eax, 16
        mov ebx, y
        idiv ebx
        mov x, eax
    }

    return x;
#endif
}

#if !defined(INLINE_ASM)
#   define fx_rcp(x) fx_div(FX_ONE, x)
#else
static inline fx_t fx_rcp(fx_t x) {
    __asm {
        .386
        mov edx, 1
        xor eax, eax
        mov ebx, x
        idiv ebx
        mov x, eax
    }

    return x;
}
#endif

//

static inline fx_t fx_sqrt(fx_t v) {
    uint32_t r = (uint32_t)v;
    uint32_t b = 0x40000000UL;
    uint32_t q = 0;
    while (b > 0x40UL) {
        uint32_t t = q + b;
        if (r >= t) {
            r -= t;
            q = t + b;
        }

        r <<= 1;
        b >>= 1;
    }

    return (fx_t)(q >> 8);
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
    uint16_t quadrant = (index >> SIN_TABLE_SIZE_SHIFT) & 3;
    uint16_t index_in_quadrant = index & SIN_TABLE_SIZE_MASK;
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
    return fx_clamp(y0 + (imul32(y1 - y0, t) >> SIN_TABLE_FX_FRACTION_BITS), -FX_ONE, FX_ONE);
}

static inline fx_t fx_cos(fx_t x) {
    return fx_sin(x + FX_ONE / 4);
}

//

static inline fx2_t fx2_rotate(fx2_t v, fx_t c, fx_t s) {
    fx2_t r;
    r.x = fx_mul_ptr(&v.x, &c) - fx_mul_ptr(&v.y, &s);
    r.y = fx_mul_ptr(&v.y, &c) + fx_mul_ptr(&v.x, &s);
    return r;
}

//

static inline void fx3_neg(fx3_t* r, const fx3_t* v) {
    r->x = -v->x;
    r->y = -v->y;
    r->z = -v->z;
}

static inline void fx3_add(fx3_t* r, const fx3_t* a, const fx3_t* b) {
    r->x = a->x + b->x;
    r->y = a->y + b->y;
    r->z = a->z + b->z;
}

static inline void fx3_add_ip(fx3_t* a, const fx3_t* b) {
    // In-place
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

static inline void fx3_sub(fx3_t* r, const fx3_t* a, const fx3_t* b) {
    r->x = a->x - b->x;
    r->y = a->y - b->y;
    r->z = a->z - b->z;
}

static inline void fx3_sub_ip(fx3_t* a, const fx3_t* b) {
    // In-place
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

static inline fx_t fx3_length_squared(const fx3_t* v) {
    return fx_pow2(v->x) + fx_pow2(v->y) + fx_pow2(v->z);
}

static inline fx_t fx3_distance_squared(const fx3_t* a, const fx3_t* b) {
    fx3_t v;
    fx3_sub(&v, a, b);
    return fx_pow2(v.x) + fx_pow2(v.y) + fx_pow2(v.z);
}

static inline fx_t fx3_length(const fx3_t* v) {
    return fx_sqrt(fx3_length_squared(v));
}

static inline fx_t fx3_length_rcp(const fx3_t* v) {
    fx_t length = fx3_length(v);
    return length != 0 ? fx_rcp(length) : FX_MAX;
}

static inline fx_t fx3_dot(const fx3_t* a, const fx3_t* b) {
    return fx_mul_ptr(&a->x, &b->x) + fx_mul_ptr(&a->y, &b->y) + fx_mul_ptr(&a->z, &b->z);
}

static inline void fx3_cross(fx3_t* r, const fx3_t* a, const fx3_t* b) {
    r->x = fx_mul_ptr(&a->y, &b->z) - fx_mul_ptr(&a->z, &b->y);
    r->y = fx_mul_ptr(&a->z, &b->x) - fx_mul_ptr(&a->x, &b->z);
    r->z = fx_mul_ptr(&a->x, &b->y) - fx_mul_ptr(&a->y, &b->x);
}

static inline void fx3_mul(fx3_t* r, const fx3_t* a, fx_t b) {
    r->x = fx_mul(a->x, b);
    r->y = fx_mul(a->y, b);
    r->z = fx_mul(a->z, b);
}

static inline void fx3_mul_ip(fx3_t* a, fx_t b) {
    // In-place
    a->x = fx_mul(a->x, b);
    a->y = fx_mul(a->y, b);
    a->z = fx_mul(a->z, b);
}

static inline void fx3_mul_cw(fx3_t* r, const fx3_t* a, const fx3_t* b) {
    // Component-wise
    r->x = fx_mul_ptr(&a->x, &b->x);
    r->y = fx_mul_ptr(&a->y, &b->y);
    r->z = fx_mul_ptr(&a->z, &b->z);
}

static inline void fx3_mul_cw_ip(fx3_t* a, const fx3_t* b) {
    // Component-wise, in-place
    a->x = fx_mul_ptr(&a->x, &b->x);
    a->y = fx_mul_ptr(&a->y, &b->y);
    a->z = fx_mul_ptr(&a->z, &b->z);
}

static inline void fx3_normalize(fx3_t* r, const fx3_t* a) {
    fx3_mul(r, a, fx3_length_rcp(a));
}

static inline void fx3_normalize_ip(fx3_t* a) {
    fx3_mul_ip(a, fx3_length_rcp(a));
}

static inline void fx3_lerp(fx3_t* r, const fx3_t* a, const fx3_t* b, fx_t t) {
    r->x = fx_lerp(a->x, b->x, t);
    r->y = fx_lerp(a->y, b->y, t);
    r->z = fx_lerp(a->z, b->z, t);
}

static inline void fx3_lerp_ip(fx3_t* a, const fx3_t* b, fx_t t) {
    a->x = fx_lerp(a->x, b->x, t);
    a->y = fx_lerp(a->y, b->y, t);
    a->z = fx_lerp(a->z, b->z, t);
}

//

static inline fx_t fx4_length_squared(const fx4_t* v) {
    return fx_pow2(v->x) + fx_pow2(v->y) + fx_pow2(v->z) + fx_pow2(v->w);
}

static inline fx_t fx4_length(const fx4_t* v) {
    return fx_sqrt(fx4_length_squared(v));
}

static inline fx_t fx4_length_rcp(const fx4_t* v) {
    fx_t length = fx_sqrt(fx4_length_squared(v));
    return length != 0 ? fx_rcp(length) : FX_MAX;
}

static inline void fx4_mul(fx4_t* r, const fx4_t* a, fx_t b) {
    r->x = fx_mul(a->x, b);
    r->y = fx_mul(a->y, b);
    r->z = fx_mul(a->z, b);
    r->w = fx_mul(a->w, b);
}

static inline void fx4_mul_ip(fx4_t* a, fx_t b) {
    // In-place
    a->x = fx_mul(a->x, b);
    a->y = fx_mul(a->y, b);
    a->z = fx_mul(a->z, b);
    a->w = fx_mul(a->w, b);
}

static inline void fx4_normalize(fx4_t* r, const fx4_t* a) {
    fx4_mul(r, a, fx4_length_rcp(a));
}

static inline void fx4_normalize_ip(fx4_t* v) {
    fx4_mul_ip(v, fx4_length_rcp(v));
}

static inline void fx_quat_normalize(fx4_t* q) {
    fx_t length_squared = fx4_length_squared(q);
    if (length_squared < FX_EPSILON_SQUARED) {
        q->x = 0;
        q->y = 0;
        q->z = 0;
        q->w = FX_ONE;
        return;
    }

    fx4_mul_ip(q, fx_rcp(fx_sqrt(length_squared)));
}

//

static inline void fx_quat_rotation_axis_angle(fx4_t* q, 
    const fx3_t* axis, fx_t angle) {
    fx_t half_angle = angle >> 1;
    fx_t factor = fx_sin(half_angle);
    q->x = fx_mul(axis->x, factor);
    q->y = fx_mul(axis->y, factor);
    q->z = fx_mul(axis->z, factor);
    q->w = fx_cos(half_angle);
    fx4_normalize_ip(q);
}

static inline void fx_quat_mul(fx4_t* r, const fx4_t* a, const fx4_t* b) {
    r->x =  fx_mul_ptr(&a->x, &b->w) +
            fx_mul_ptr(&a->y, &b->z) -
            fx_mul_ptr(&a->z, &b->y) +
            fx_mul_ptr(&a->w, &b->x);
    r->y = -fx_mul_ptr(&a->x, &b->z) +
            fx_mul_ptr(&a->y, &b->w) +
            fx_mul_ptr(&a->z, &b->x) +
            fx_mul_ptr(&a->w, &b->y);
    r->z =  fx_mul_ptr(&a->x, &b->y) -
            fx_mul_ptr(&a->y, &b->x) +
            fx_mul_ptr(&a->z, &b->w) +
            fx_mul_ptr(&a->w, &b->z);
    r->w = -fx_mul_ptr(&a->x, &b->x) -
            fx_mul_ptr(&a->y, &b->y) -
            fx_mul_ptr(&a->z, &b->z) +
            fx_mul_ptr(&a->w, &b->w);
}

static inline void fx_quat_mul_ip(fx4_t* a, const fx4_t* b) {
    fx4_t t;
    fx_quat_mul(&t, a, b);
    *a = t;
}

//

#define FX3X3_00 0
#define FX3X3_01 1
#define FX3X3_02 2
#define FX3X3_10 3
#define FX3X3_11 4
#define FX3X3_12 5
#define FX3X3_20 6
#define FX3X3_21 7
#define FX3X3_22 8

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

static inline void fx4x3_translation(fx4x3_t* m, const fx3_t* translation) {
    m->m[0] = FX_ONE;
    m->m[1] = 0;
    m->m[2] = 0;
    m->m[3] = 0;
    m->m[4] = FX_ONE;
    m->m[5] = 0;
    m->m[6] = 0;
    m->m[7] = 0;
    m->m[8] = FX_ONE;
    m->m[9] = translation->x;
    m->m[10] = translation->y;
    m->m[11] = translation->z;
}

static inline void fx4x3_set_translation(fx4x3_t* m, const fx3_t* translation) {
    m->m[9] = translation->x;
    m->m[10] = translation->y;
    m->m[11] = translation->z;
}

static inline void fx3x3_rotation(fx3x3_t* m, const fx4_t* q) {
    fx_t xx = fx_pow2(q->x);
    fx_t xy = fx_mul_ptr(&q->x, &q->y);
    fx_t xz = fx_mul_ptr(&q->x, &q->z);
    fx_t xw = fx_mul_ptr(&q->x, &q->w);

    fx_t yy = fx_pow2(q->y);
    fx_t yz = fx_mul_ptr(&q->y, &q->z);
    fx_t yw = fx_mul_ptr(&q->y, &q->w);

    fx_t zz = fx_pow2(q->z);
    fx_t zw = fx_mul_ptr(&q->z, &q->w);

    m->m[FX3X3_00] = fx_clamp(FX_ONE - 2 * (yy + zz), -FX_ONE, FX_ONE);
    m->m[FX3X3_01] = fx_clamp(         2 * (xy - zw), -FX_ONE, FX_ONE);
    m->m[FX3X3_02] = fx_clamp(         2 * (xz + yw), -FX_ONE, FX_ONE);

    m->m[FX3X3_10] = fx_clamp(         2 * (xy + zw), -FX_ONE, FX_ONE);
    m->m[FX3X3_11] = fx_clamp(FX_ONE - 2 * (xx + zz), -FX_ONE, FX_ONE);
    m->m[FX3X3_12] = fx_clamp(         2 * (yz - xw), -FX_ONE, FX_ONE);

    m->m[FX3X3_20] = fx_clamp(         2 * (xz - yw), -FX_ONE, FX_ONE);
    m->m[FX3X3_21] = fx_clamp(         2 * (yz + xw), -FX_ONE, FX_ONE);
    m->m[FX3X3_22] = fx_clamp(FX_ONE - 2 * (xx + yy), -FX_ONE, FX_ONE);
}

static inline void fx4x3_rotation_translation(fx4x3_t* m, const fx4_t* q,
    const fx3_t* translation) {
    fx_t xx = fx_pow2(q->x);
    fx_t xy = fx_mul_ptr(&q->x, &q->y);
    fx_t xz = fx_mul_ptr(&q->x, &q->z);
    fx_t xw = fx_mul_ptr(&q->x, &q->w);

    fx_t yy = fx_pow2(q->y);
    fx_t yz = fx_mul_ptr(&q->y, &q->z);
    fx_t yw = fx_mul_ptr(&q->y, &q->w);

    fx_t zz = fx_pow2(q->z);
    fx_t zw = fx_mul_ptr(&q->z, &q->w);

    m->m[FX4X3_00] = fx_clamp(FX_ONE - 2 * (yy + zz), -FX_ONE, FX_ONE);
    m->m[FX4X3_01] = fx_clamp(         2 * (xy - zw), -FX_ONE, FX_ONE);
    m->m[FX4X3_02] = fx_clamp(         2 * (xz + yw), -FX_ONE, FX_ONE);

    m->m[FX4X3_10] = fx_clamp(         2 * (xy + zw), -FX_ONE, FX_ONE);
    m->m[FX4X3_11] = fx_clamp(FX_ONE - 2 * (xx + zz), -FX_ONE, FX_ONE);
    m->m[FX4X3_12] = fx_clamp(         2 * (yz - xw), -FX_ONE, FX_ONE);

    m->m[FX4X3_20] = fx_clamp(         2 * (xz - yw), -FX_ONE, FX_ONE);
    m->m[FX4X3_21] = fx_clamp(         2 * (yz + xw), -FX_ONE, FX_ONE);
    m->m[FX4X3_22] = fx_clamp(FX_ONE - 2 * (xx + yy), -FX_ONE, FX_ONE);

    m->m[FX4X3_30] = translation->x;
    m->m[FX4X3_31] = translation->y;
    m->m[FX4X3_32] = translation->z;
}

static inline void fx_transform_point_ip(const fx4x3_t* m, fx3_t* v) {
    int32_t x, y, z;
#if !defined(INLINE_ASM)
    x = fx_mul(v->x, m->m[FX4X3_00]) + fx_mul(v->y, m->m[FX4X3_01]) + fx_mul(v->z, m->m[FX4X3_02]) + m->m[FX4X3_30];
    y = fx_mul(v->x, m->m[FX4X3_10]) + fx_mul(v->y, m->m[FX4X3_11]) + fx_mul(v->z, m->m[FX4X3_12]) + m->m[FX4X3_31];
    z = fx_mul(v->x, m->m[FX4X3_20]) + fx_mul(v->y, m->m[FX4X3_21]) + fx_mul(v->z, m->m[FX4X3_22]) + m->m[FX4X3_32];
#else
    __asm {
        .386
        mov si, v
        mov di, m

        mov eax, [si]
        mov ecx, eax
        mov edx, [di]
        imul edx
        shrd eax, edx, 16
        mov x, eax

        mov eax, ecx
        mov edx, 12[di]
        imul edx
        shrd eax, edx, 16
        mov y, eax

        mov eax, ecx
        mov edx, 24[di]
        imul edx
        shrd eax, edx, 16
        mov z, eax

        //

        mov eax, 4[si]
        mov ecx, eax
        mov edx, 4[di]
        imul edx
        shrd eax, edx, 16
        add x, eax

        mov eax, ecx
        mov edx, 16[di]
        imul edx
        shrd eax, edx, 16
        add y, eax

        mov eax, ecx
        mov edx, 28[di]
        imul edx
        shrd eax, edx, 16
        add z, eax

        //

        mov eax, 8[si]
        mov ecx, eax
        mov edx, 8[di]
        imul edx
        shrd eax, edx, 16
        add x, eax

        mov eax, ecx
        mov edx, 20[di]
        imul edx
        shrd eax, edx, 16
        add y, eax

        mov eax, ecx
        mov edx, 32[di]
        imul edx
        shrd eax, edx, 16
        add z, eax

        //

        mov edx, 36[di]
        add x, edx

        mov edx, 40[di]
        add y, edx

        mov edx, 44[di]
        add z, edx
    }
#endif
    v->x = x;
    v->y = y;
    v->z = z;
}

static inline void fx_transform_point(fx3_t* r, const fx4x3_t* m, const fx3_t* v) {
    *r = *v;
    fx_transform_point_ip(m, r);
}

static inline void fx_transform_vector_ip(const fx3x3_t* m, fx3_t* v) {
    int32_t x, y, z;
#if !defined(INLINE_ASM)
    x = fx_mul(v->x, m->m[FX4X3_00]) + fx_mul(v->y, m->m[FX4X3_01]) + fx_mul(v->z, m->m[FX4X3_02]);
    y = fx_mul(v->x, m->m[FX4X3_10]) + fx_mul(v->y, m->m[FX4X3_11]) + fx_mul(v->z, m->m[FX4X3_12]);
    z = fx_mul(v->x, m->m[FX4X3_20]) + fx_mul(v->y, m->m[FX4X3_21]) + fx_mul(v->z, m->m[FX4X3_22]);
#else
    __asm {
        .386
        mov si, v
        mov di, m

        mov eax, [si]
        mov ecx, eax
        mov edx, [di]
        imul edx
        shrd eax, edx, 16
        mov x, eax

        mov eax, ecx
        mov edx, 12[di]
        imul edx
        shrd eax, edx, 16
        mov y, eax

        mov eax, ecx
        mov edx, 24[di]
        imul edx
        shrd eax, edx, 16
        mov z, eax

        //

        mov eax, 4[si]
        mov ecx, eax
        mov edx, 4[di]
        imul edx
        shrd eax, edx, 16
        add x, eax

        mov eax, ecx
        mov edx, 16[di]
        imul edx
        shrd eax, edx, 16
        add y, eax

        mov eax, ecx
        mov edx, 28[di]
        imul edx
        shrd eax, edx, 16
        add z, eax

        //

        mov eax, 8[si]
        mov ecx, eax
        mov edx, 8[di]
        imul edx
        shrd eax, edx, 16
        add x, eax

        mov eax, ecx
        mov edx, 20[di]
        imul edx
        shrd eax, edx, 16
        add y, eax

        mov eax, ecx
        mov edx, 32[di]
        imul edx
        shrd eax, edx, 16
        add z, eax
    }
#endif
    v->x = x;
    v->y = y;
    v->z = z;
}

static inline void fx_transform_vector(fx3_t* r, const fx3x3_t* m, const fx3_t* v) {
    *r = *v;
    fx_transform_vector_ip(m, r);
}

static inline void fx4x3_look_at(fx4x3_t* m, const fx3_t* eye,
    const fx3_t* target, const fx3_t* up) {
    fx3_t view_forward, view_right, view_up, view_translation;

    if (eye)
        fx3_sub(&view_forward, target, eye);
    else
        view_forward = *target;

    fx3_normalize_ip(&view_forward);

    fx3_cross(&view_right, &view_forward, up);
    fx3_normalize_ip(&view_right);

    fx3_cross(&view_up, &view_right, &view_forward);
    fx3_normalize_ip(&view_up);

    m->m[FX4X3_00] = view_right.x;
    m->m[FX4X3_01] = view_right.y;
    m->m[FX4X3_02] = view_right.z;

    m->m[FX4X3_10] = view_up.x;
    m->m[FX4X3_11] = view_up.y;
    m->m[FX4X3_12] = view_up.z;

    m->m[FX4X3_20] = view_forward.x;
    m->m[FX4X3_21] = view_forward.y;
    m->m[FX4X3_22] = view_forward.z;

    if (eye) {
        fx3_neg(&view_translation, eye);
        fx_transform_vector_ip((const fx3x3_t*)m, &view_translation);
        m->m[FX4X3_30] = view_translation.x;
        m->m[FX4X3_31] = view_translation.y;
        m->m[FX4X3_32] = view_translation.z;
    } else {
        m->m[FX4X3_30] = 0;
        m->m[FX4X3_31] = 0;
        m->m[FX4X3_32] = 0;
    }
}

static inline void fx4x3_mul(fx4x3_t* r, const fx4x3_t* a, const fx4x3_t* b) {
    r->m[FX4X3_00] =
        fx_mul_ptr(&a->m[FX4X3_00], &b->m[FX4X3_00]) +
        fx_mul_ptr(&a->m[FX4X3_01], &b->m[FX4X3_10]) +
        fx_mul_ptr(&a->m[FX4X3_02], &b->m[FX4X3_20]);
    r->m[FX4X3_01] =
        fx_mul_ptr(&a->m[FX4X3_00], &b->m[FX4X3_01]) +
        fx_mul_ptr(&a->m[FX4X3_01], &b->m[FX4X3_11]) +
        fx_mul_ptr(&a->m[FX4X3_02], &b->m[FX4X3_21]);
    r->m[FX4X3_02] =
        fx_mul_ptr(&a->m[FX4X3_00], &b->m[FX4X3_02]) +
        fx_mul_ptr(&a->m[FX4X3_01], &b->m[FX4X3_12]) +
        fx_mul_ptr(&a->m[FX4X3_02], &b->m[FX4X3_22]);
    r->m[FX4X3_10] =
        fx_mul_ptr(&a->m[FX4X3_10], &b->m[FX4X3_00]) +
        fx_mul_ptr(&a->m[FX4X3_11], &b->m[FX4X3_10]) +
        fx_mul_ptr(&a->m[FX4X3_12], &b->m[FX4X3_20]);
    r->m[FX4X3_11] =
        fx_mul_ptr(&a->m[FX4X3_10], &b->m[FX4X3_01]) +
        fx_mul_ptr(&a->m[FX4X3_11], &b->m[FX4X3_11]) +
        fx_mul_ptr(&a->m[FX4X3_12], &b->m[FX4X3_21]);
    r->m[FX4X3_12] =
        fx_mul_ptr(&a->m[FX4X3_10], &b->m[FX4X3_02]) +
        fx_mul_ptr(&a->m[FX4X3_11], &b->m[FX4X3_12]) +
        fx_mul_ptr(&a->m[FX4X3_12], &b->m[FX4X3_22]);
    r->m[FX4X3_20] =
        fx_mul_ptr(&a->m[FX4X3_20], &b->m[FX4X3_00]) +
        fx_mul_ptr(&a->m[FX4X3_21], &b->m[FX4X3_10]) +
        fx_mul_ptr(&a->m[FX4X3_22], &b->m[FX4X3_20]);
    r->m[FX4X3_21] =
        fx_mul_ptr(&a->m[FX4X3_20], &b->m[FX4X3_01]) +
        fx_mul_ptr(&a->m[FX4X3_21], &b->m[FX4X3_11]) +
        fx_mul_ptr(&a->m[FX4X3_22], &b->m[FX4X3_21]);
    r->m[FX4X3_22] =
        fx_mul_ptr(&a->m[FX4X3_20], &b->m[FX4X3_02]) +
        fx_mul_ptr(&a->m[FX4X3_21], &b->m[FX4X3_12]) +
        fx_mul_ptr(&a->m[FX4X3_22], &b->m[FX4X3_22]);

    fx_transform_point((fx3_t*)&r->m[FX4X3_30], a, (const fx3_t*)&b->m[FX4X3_30]);
}

#endif
