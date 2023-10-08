#ifndef OPL_H
#define OPL_H

#include "opl_bank.h"

#define OPL_BASE 0x388

#define NUM_OPL_CHANNELS 9
#define NUM_OPL_MUSIC_CHANNELS 8
#define OPL_SFX_CHANNEL 8

static const opl_channel_offsets[NUM_OPL_CHANNELS] = {
    0x0, 0x1, 0x2, 0x8, 0x9, 0xa, 0x10, 0x11, 0x12
};

static volatile uint8_t opl_register_states[256] = { 0 };

#if defined(INLINE_ASM)

void opl_write_asm(uint8_t reg, uint8_t v);
#pragma aux opl_write_asm = \
"mov dx, 388h" \
"out dx, al" \
"mov cx, 6" \
"wait_index:" \
"in al, dx" \
"loop wait_index" \
"inc dx" \
"mov al, bl" \
"out dx, al" \
"mov cx, 35" \
"wait_data:" \
"in al, dx" \
"loop wait_data" \
modify [al bl cx dx] \
parm [al] [bl];

static inline void opl_write(uint8_t reg, uint8_t v) {
    opl_register_states[reg] = v;
    opl_write_asm(reg, v);
}

static inline void opl_write_fast(uint8_t reg, uint8_t v) {
    if (opl_register_states[reg] == v)
        return;

    opl_register_states[reg] = v;
    opl_write_asm(reg, v);
}

#else

static inline void opl_wait_index() {
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
}

static inline void opl_wait_data() {
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
    inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE); inp(OPL_BASE);
}

static inline void opl_write(uint8_t reg, uint8_t v) {
    opl_register_states[reg] = v;

    outp(OPL_BASE, reg);
    opl_wait_index();
    outp(OPL_BASE + 1, v);
    opl_wait_data();

#if 0
    if (opl == 3) {
        outp(OPL_BASE + 2, reg);
        opl_wait_index();
        outp(OPL_BASE + 2 + 1, v);
        opl_wait_data();
    }
#endif
}

static inline void opl_write_fast(uint8_t reg, uint8_t v) {
    if (opl_register_states[reg] == v)
        return;

    opl_register_states[reg] = v;

    outp(OPL_BASE, reg);
    opl_wait_index();
    outp(OPL_BASE + 1, v);
    opl_wait_data();
}
#endif

static inline void opl_reset() {
    uint8_t i;

    if (!opl)
        return;

    for (i = 0x01; i <= 0xf5; i++) {
        opl_write(i, 0);
    }
}

static inline void opl_init() {
    uint8_t val1, val2;

    // Reset timer 1 and 2
    opl_write(0x4, 0x60);

    // Reset IRQ
    opl_write(0x4, 0x80);

    // Read status
    val1 = inp(OPL_BASE);

    // Set timer 1 to 0xff
    opl_write(0x2, 0xff);

    // Start timer 1
    opl_write(0x4, 0x21);

    // Delay for more than 80us, pick 10ms
    delay(10);

    // Read status
    val2 = inp(OPL_BASE);

    // Reset timer 1 and 2
    opl_write(0x4, 0x60);

    // Reset IRQ
    opl_write(0x4, 0x80);

    if ((val1 & 0xe0) != 0x00 || (val2 & 0xe0) != 0xc0) {
        return;
    }

    val1 = inp(OPL_BASE);

    if ((val1 & 0x06) == 0x00) {
        opl = 3;
    } else {
        opl = 2;
    }

    opl_reset();
    opl_write(0x01, 0x20);
    opl_write(0x08, 0x40);
    opl_write(0xbd, 0x80 | 0x40);
}

static inline void opl_done() {
    if (!opl)
        return;

    opl_reset();
    opl = 0;
}

static inline void opl_stop(uint8_t channel) {
    uint8_t reg;

    if (!opl)
        return;

    reg = 0xb0 + channel;
    opl_write(reg, opl_register_states[reg] & ~0x20); // Voice off
}

static const uint16_t opl_freq_table[12] = {
    342, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647
};

static const uint8_t opl_attenuation_table[128] = {
    0x3F, 0x38, 0x30, 0x2B, 0x28, 0x25, 0x23, 0x21,
    0x20, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18,
    0x17, 0x17, 0x16, 0x16, 0x15, 0x14, 0x14, 0x13,
    0x13, 0x12, 0x12, 0x11, 0x11, 0x11, 0x10, 0x10,
    0x0F, 0x0F, 0x0F, 0x0E, 0x0E, 0x0E, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0A, 0x0A, 0x0A, 0x0A, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 
    0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static inline void opl_play(uint8_t channel, uint8_t program, uint8_t note, uint8_t velocity) {
    aw_assert(channel < NUM_OPL_CHANNELS && program < 128 && note < 128 &&
        velocity < 128);

    if (!opl)
        return;

    {
        const uint8_t* inst_data = opl_bank[program];
        uint8_t ch_offset = opl_channel_offsets[channel];

        opl_write_fast(0x20 + ch_offset, inst_data[0]);
        opl_write_fast(0x23 + ch_offset, inst_data[1]);

        {
            uint8_t attenuation0, attenuation1;
            uint8_t attenuation = opl_attenuation_table[velocity];

            if (inst_data[10] & 1) { // If AM
                attenuation0 = (inst_data[2] & 0x3f) + attenuation;
                if (attenuation0 > 0x3f)
                    attenuation0 = 0x3f;
                opl_write_fast(0x40 + ch_offset, attenuation0 | (inst_data[2] & 0xc0));
            } else {
                opl_write_fast(0x40 + ch_offset, inst_data[2]);
            }

            attenuation1 = (inst_data[3] & 0x3f) + attenuation;
            if (attenuation1 > 0x3f)
                attenuation1 = 0x3f;

            opl_write_fast(0x43 + ch_offset, attenuation1 | (inst_data[3] & 0xc0));
        }

        opl_write_fast(0x60 + ch_offset, inst_data[4]);
        opl_write_fast(0x63 + ch_offset, inst_data[5]);
        opl_write_fast(0x80 + ch_offset, inst_data[6]);
        opl_write_fast(0x83 + ch_offset, inst_data[7]);
        opl_write_fast(0xe0 + ch_offset, inst_data[8]);
        opl_write_fast(0xe3 + ch_offset, inst_data[9]);
        opl_write_fast(0xc0 + channel, inst_data[10]);
    }

    {
        uint8_t octave = 0;
        uint16_t f;

        while (note >= 12) {
            note -= 12;
            octave++;
        }

        f = opl_freq_table[note];

        // Set voice frequency's LSB
        opl_write_fast(0xa0 + channel, f & 0xff);

        // Turn the voice on; set the octave and freq MSB
        opl_write(0xb0 + channel, 0x20 | (octave << 2) | (f >> 8));
    }
}

static inline void opl_warmup() {
    uint8_t i;
    for (i = 0; i < NUM_OPL_CHANNELS; ++i) {
        opl_play(i, 122, 127, 0);
    }

    for (i = 0; i < NUM_OPL_CHANNELS; ++i) {
        opl_stop(i);
    }
}

#endif
