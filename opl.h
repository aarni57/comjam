#ifndef OPL_H
#define OPL_H

static uint8_t opl_register_cache[256] = { 0 };

#if 0

void opl_write(uint8_t reg, uint8_t v);
#pragma aux opl_write = \
"mov dx, word ptr opl_base" \
"out dx, al" \
"in al, dx" \
"inc dx" \
"mov al, bl" \
"out dx, al" \
"in al, dx" \
modify [dx] \
parm [al] [bl];

#else

static inline void opl_wait_index() {
    inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base);
}

static inline void opl_wait_data() {
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
    inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base); inp(opl_base);
}

static inline void opl_write(uint8_t reg, uint8_t v) {
    opl_register_cache[reg] = v;

    outp(opl_base, reg);
    opl_wait_index();
    outp(opl_base + 1, v);
    opl_wait_data();

#if 0
    if (opl == 3) {
        outp(opl_base + 2, reg);
        opl_wait_index();
        outp(opl_base + 2 + 1, v);
        opl_wait_data();
    }
#endif
}

static inline void opl_write_fast(uint8_t reg, uint8_t v) {
    if (opl_register_cache[reg] == v)
        return;

    opl_register_cache[reg] = v;

    outp(opl_base, reg);
    opl_wait_index();
    outp(opl_base + 1, v);
    opl_wait_data();
}
#endif

static void opl_reset() {
    uint8_t i;

    if (!opl)
        return;

    for (i = 0x01; i <= 0xf5; i++) {
        opl_write(i, 0);
    }
}

static void opl_init() {
    uint8_t val1, val2;

    // Reset timer 1 and 2
    opl_write(0x4, 0x60);

    // Reset IRQ
    opl_write(0x4, 0x80);

    // Read status
    val1 = inp(opl_base);

    // Set timer 1 to 0xff
    opl_write(0x2, 0xff);

    // Start timer 1
    opl_write(0x4, 0x21);

    // Delay for more than 80us, pick 10ms
    delay(10);

    // Read status
    val2 = inp(opl_base);

    // Reset timer 1 and 2
    opl_write(0x4, 0x60);

    // Reset IRQ
    opl_write(0x4, 0x80);

    if ((val1 & 0xe0) != 0x00 || (val2 & 0xe0) != 0xc0) {
        return;
    }

#if 0
    val1 = inp(opl_base);

    if ((val1 & 0x06) == 0x00) {
        opl = 3;
    } else {
        opl = 2;
    }
#else
    opl = 2;
#endif

    opl_reset();
    opl_write(0x01, 0x20);
    opl_write(0x08, 0x40);
}

static void opl_done() {
    if (!opl)
        return;

    opl_reset();
    opl = 0;
}

static void opl_play() {
    uint8_t voice = 0;

    if (!opl)
        return;

    opl_write(0xb0, 0); // Voice off

    opl_write_fast(0x20, 0x01); // Set the modulator's multiple
    opl_write_fast(0x40, 0x06); // Set the modulator's level
    opl_write_fast(0x60, 0x11); // Modulator attack & decay
    opl_write_fast(0x80, 0x11); // Modulator sustain & release
    opl_write_fast(0xa0, 0x28); // Set voice frequency's LSB
    opl_write_fast(0x23, 0x00); // Set the carrier's multiple
    opl_write_fast(0x43, 0x00); // Set the carrier to maximum volume
    opl_write_fast(0x63, 0x11); // Carrier attack & decay
    opl_write_fast(0x83, 0x11); // Carrier sustain & release
    opl_write(0xb0, 0x20 | 0x10); // Turn the voice on; set the octave and freq MSB
}

#endif
