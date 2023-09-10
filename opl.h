#ifndef OPL_H
#define OPL_H

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

#if 0
static inline void opl_write(uint8_t reg, uint8_t v) {
    outp(opl_base, reg);
    inp(opl_base);
    outp(opl_base + 1, v);
    inp(opl_base);

#if 0
    if (opl == 3) {
        outp(opl_base + 2, reg);
        inp(opl_base);
        outp(opl_base + 2 + 1, v);
        inp(opl_base);
    }
#endif
}
#endif

static void opl_reset() {
    uint8_t i;

    if (!opl_enabled)
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

    opl_enabled = 1;

#if 0
    // OPL3 detection (not used)
    val1 = inp(opl_base);

    if ((val1 & 0x06) == 0x00) {
        opl = 3;
    } else {
        opl = 2;
    }
#endif

    opl_reset();
    opl_write(0x01, 0x20);
    opl_write(0x08, 0x40);
}

static void opl_done() {
    if (!opl_enabled)
        return;

    opl_reset();
}

static void opl_play() {
    uint8_t voice = 0;

    if (!opl_enabled)
        return;

    opl_write(0xb0, 0);

    opl_write(0x20, 0x01); // Set the modulator's multiple
    opl_write(0x40, 0x06); // Set the modulator's level
    opl_write(0x60, 0x11); // Modulator attack & decay
    opl_write(0x80, 0x11); // Modulator sustain & release
    opl_write(0xa0, 0x28); // Set voice frequency's LSB
    opl_write(0x23, 0x00); // Set the carrier's multiple
    opl_write(0x43, 0x00); // Set the carrier to maximum volume
    opl_write(0x63, 0x11); // Carrier attack & decay
    opl_write(0x83, 0x11); // Carrier sustain & release
    opl_write(0xb0, 0x20 | 0x10); // Turn the voice on; set the octave and freq MSB
}

#endif
