#if !defined(INLINE_ASM)
#define XORSHIFT32_INITIAL_VALUE 0xcafebabe
static uint32_t xorshift32_state = XORSHIFT32_INITIAL_VALUE;

static inline uint32_t xorshift32() {
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    xorshift32_state ^= xorshift32_state << 13;
    xorshift32_state ^= xorshift32_state >> 17;
    xorshift32_state ^= xorshift32_state << 5;
    return xorshift32_state;
}

static inline void xorshift32_reset() {
    xorshift32_state = XORSHIFT32_INITIAL_VALUE;
}
#else
uint32_t xorshift32();
void xorshift32_reset();
#endif

#if !defined(INLINE_ASM)
static inline fx_t fx_random_one() {
    return xorshift32() & (FX_ONE - 1);
}

static inline fx_t fx_random_signed_one() {
    return (xorshift32() & (FX_ONE * 2 - 1)) - FX_ONE;
}

static inline fx_t random_debris_x() {
    return fx_random_signed_one() >> 1;
}
#else
fx_t fx_random_one();
fx_t fx_random_signed_one();
fx_t random_debris_x();
#endif
