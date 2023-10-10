#include "keys.h"

#define INVALID_KEY 0xff

static uint8_t translate_key(uint32_t code) {
    switch (code) {
        case 0x01: return KEY_ESC;
        case 0x1c: return KEY_ENTER;
        case 0x39: return KEY_SPACE;
        case 0x0e: return KEY_BACKSPACE;
        case 0x0f: return KEY_TAB;

        case 0x2a: return KEY_LSHIFT;
        case 0x36: return KEY_RSHIFT;
        case 0x1d: return KEY_LCTRL;
        case 0x38: return KEY_LALT;

        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4b: return KEY_LEFT;
        case 0x4d: return KEY_RIGHT;

        case 0x02: return KEY_1;
        case 0x03: return KEY_2;
        case 0x04: return KEY_3;
        case 0x05: return KEY_4;
        case 0x06: return KEY_5;
        case 0x07: return KEY_6;
        case 0x08: return KEY_7;
        case 0x09: return KEY_8;
        case 0x0a: return KEY_9;
        case 0x0b: return KEY_0;

        case 0x1e: return KEY_A;
        case 0x30: return KEY_B;
        case 0x2e: return KEY_C;
        case 0x20: return KEY_D;
        case 0x12: return KEY_E;
        case 0x21: return KEY_F;
        case 0x22: return KEY_G;
        case 0x23: return KEY_H;
        case 0x17: return KEY_I;
        case 0x24: return KEY_J;
        case 0x25: return KEY_K;
        case 0x26: return KEY_L;
        case 0x32: return KEY_M;
        case 0x31: return KEY_N;
        case 0x18: return KEY_O;
        case 0x19: return KEY_P;
        case 0x10: return KEY_Q;
        case 0x13: return KEY_R;
        case 0x1f: return KEY_S;
        case 0x14: return KEY_T;
        case 0x16: return KEY_U;
        case 0x2f: return KEY_V;
        case 0x11: return KEY_W;
        case 0x2d: return KEY_X;
        case 0x15: return KEY_Y;
        case 0x2c: return KEY_Z;

        case 0x3b: return KEY_F1;
        case 0x3c: return KEY_F2;
        case 0x3d: return KEY_F3;
        case 0x3e: return KEY_F4;
        case 0x3f: return KEY_F5;
        case 0x40: return KEY_F6;
        case 0x41: return KEY_F7;
        case 0x42: return KEY_F8;
        case 0x43: return KEY_F9;
        case 0x44: return KEY_F10;
        case 0x57: return KEY_F11;
        case 0x58: return KEY_F12;

        case 0x29: return KEY_TILDE;

        case 0x33: return KEY_COMMA;
        case 0x34: return KEY_PERIOD;
        case 0x35: return KEY_HYPHEN;
        case 0x2b: return KEY_ASTERISK;
        case 0x0c: return KEY_PLUS;
        case 0x56: return KEY_ANGLE_BRACKETS;

        case 0x11c: return KEY_KEYPAD_ENTER;

        case 0x11d: return KEY_RCTRL;
        case 0x138: return KEY_RALT;

        case 0x148: return KEY_GREY_UP;
        case 0x150: return KEY_GREY_DOWN;
        case 0x14b: return KEY_GREY_LEFT;
        case 0x14d: return KEY_GREY_RIGHT;

        case 0x152: return KEY_INSERT;
        case 0x153: return KEY_DELETE;
        case 0x147: return KEY_HOME;
        case 0x14f: return KEY_END;
        case 0x149: return KEY_PAGE_UP;
        case 0x151: return KEY_PAGE_DOWN;

        default: return INVALID_KEY;
    }
}

#define KEY_BUFFER_SIZE 64
static volatile uint8_t key_buffer[KEY_BUFFER_SIZE];
static volatile uint8_t key_buffer_position = 0;
static volatile uint8_t extended_key = 0;

void keyb_key(uint8_t code) {
    if (code != 0xe0) {
        uint8_t key = translate_key((code & 0x7f) | (extended_key ? 0x100 : 0));
        if (key != INVALID_KEY) {
            int up = (code & 0x80) == 0x80;
            aw_assert(key_buffer_position < KEY_BUFFER_SIZE);
            key_buffer[key_buffer_position] = key | (up ? KEY_UP_FLAG : 0);
            key_buffer_position = (key_buffer_position + 1) & (KEY_BUFFER_SIZE - 1);
        }

        extended_key = 0;
    } else {
        extended_key = 1;
    }
}

// defined in keyb.asm
void keyb_init();
void keyb_cleanup();
