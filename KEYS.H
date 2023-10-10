
// AW library
// Aarni Gratseff (aarni.gratseff@gmail.com)
// Keys

#ifndef AW_KEYS_H
#define AW_KEYS_H

enum {
    KEY_NULL,

    KEY_ESC,
    KEY_ENTER,
    KEY_SPACE,
    KEY_BACKSPACE,
    KEY_TAB,

    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_LALT,
    KEY_RALT,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_GREY_UP,
    KEY_GREY_DOWN,
    KEY_GREY_LEFT,
    KEY_GREY_RIGHT,

    KEY_KEYPAD_ENTER,

    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_TILDE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_HYPHEN,
    KEY_ASTERISK,
    KEY_PLUS,
    KEY_ANGLE_BRACKETS,

    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,

    NUM_KEYS
};

#define KEY_UP_FLAG 0x80
#define KEY_INDEX_MASK 0x7f

#define KEY_STATE_DOWN 1
#define KEY_STATE_DOWN_REPEAT 2
#define KEY_STATE_CHANGED_TO_DOWN 4
#define KEY_STATE_PRESSED 8

#define KEY_MODIFIER_CTRL 16
#define KEY_MODIFIER_ALT 32
#define KEY_MODIFIER_SHIFT 64

#endif // AW_KEYS_H
