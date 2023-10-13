#ifndef UTIL_H
#define UTIL_H

#include <conio.h>
#include <stdint.h>

static inline void kb_clear_buffer();
#pragma aux kb_clear_buffer =   \
"mov ax, 0x0c00" \
"int 0x21" \
modify [ax];

static inline void putz(const char* str);
#pragma aux putz = \
"l:" \
"mov dl, [bx]" \
"test dl, dl" \
"jz end" \
"push bx" \
"mov ah, 02h" \
"int 21h" \
"pop bx" \
"add bx, 1" \
"jmp l" \
"end:" \
modify [ax dx] \
parm [bx];

static inline void set_text_cursor(uint8_t row, uint8_t col);
#pragma aux set_text_cursor = \
"mov ah, 2" \
"mov bh, 0" \
"xor al, al" \
"int 10h" \
modify [ax bh] \
parm [dh] [dl];

static inline void split_number(uint16_t v, uint8_t* o, uint8_t* te, uint8_t* h, uint8_t* th, uint8_t* tt) {
    uint8_t ones, tens, hundreds, thousands, tens_thousands;
#if !defined(INLINE_ASM)
    tens_thousands = v / 10000;
    v -= tens_thousands * 10000;
    thousands = v / 1000;
    v -= thousands * 1000;
    hundreds = v / 100;
    v -= hundreds * 100;
    tens = v / 10;
    v -= tens * 10;
    ones = v;
#else
    __asm {
        .386
        mov ax, v
        mov cx, 10000
        xor dx, dx
        div cx
        mov tens_thousands, al
        mov ax, dx
        mov cx, 1000
        xor dx, dx
        div cx
        mov thousands, al
        mov ax, dx
        mov cx, 100
        xor dx, dx
        div cx
        mov hundreds, al
        mov ax, dx
        mov cx, 10
        xor dx, dx
        div cx
        mov tens, al
        mov ones, dl
    }
#endif
    *o = ones;
    *te = tens;
    *h = hundreds;
    *th = thousands;
    *tt = tens_thousands;
}

static inline char number_to_char(uint8_t v) {
    return '0' + v;
}

static inline void number_to_string(char* buf, uint16_t v) {
    uint8_t ones, tens, hundreds, thousands, tens_thousands;
    char* tgt = buf;
    split_number(v, &ones, &tens, &hundreds, &thousands, &tens_thousands);
    if (tens_thousands) *tgt++ = number_to_char(tens_thousands);
    if (tens_thousands || thousands) *tgt++ = number_to_char(thousands);
    if (tens_thousands || thousands || hundreds) *tgt++ = number_to_char(hundreds);
    if (tens_thousands || thousands || hundreds || tens) *tgt++ = number_to_char(tens);
    *tgt = number_to_char(ones);
}

#endif
