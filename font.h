#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_FIRST_CHARACTER 33
#define FONT_LAST_CHARACTER 126
#define FONT_NUM_CHARACTERS (FONT_LAST_CHARACTER - FONT_FIRST_CHARACTER + 1)

typedef struct font_t {
    uint8_t width;
    uint8_t height;
    uint8_t space_width;
    uint8_t spacing;
    uint8_t line_height;
    const int8_t* spacings;
} font_t;

#define FONT6X6_WIDTH 5
#define FONT6X6_HEIGHT 6

#define FONT6X6_SPACE_WIDTH 4
#define FONT6X6_SPACING 5
#define FONT6X6_LINE_HEIGHT 7

const int8_t FONT6X6_SPACINGS[] = {
    -3, /* ! */   -1, /* " */    1, /* # */    0, /* $ */    1, /* % */
     0, /* & */   -2, /* ' */   -1, /* ( */   -1, /* ) */   -1, /* * */
    -1, /* + */   -3, /* , */   -2, /* - */   -3, /* . */   -1, /* / */
     0, /* 0 */   -2, /* 1 */    0, /* 2 */    0, /* 3 */    0, /* 4 */
     0, /* 5 */    0, /* 6 */    0, /* 7 */    0, /* 8 */    0, /* 9 */
    -3, /* : */   -3, /* ; */   -1, /* < */    0, /* = */   -1, /* > */
     0, /* ? */    0, /* @ */    0, /* A */    0, /* B */    0, /* C */
     0, /* D */    0, /* E */    0, /* F */    0, /* G */    0, /* H */
    -1, /* I */    0, /* J */    0, /* K */    0, /* L */    1, /* M */
     0, /* N */    0, /* O */    0, /* P */    0, /* Q */    0, /* R */
     0, /* S */   -1, /* T */    0, /* U */    0, /* V */    1, /* W */
     0, /* X */    0, /* Y */    0, /* Z */   -2, /* [ */    0, /* \ */
    -2, /* ] */    0, /* ^ */   -1, /* _ */   -2, /* ` */    0, /* a */
     0, /* b */    0, /* c */    0, /* d */    0, /* e */   -1, /* f */
     0, /* g */    0, /* h */   -3, /* i */   -1, /* j */    0, /* k */
    -3, /* l */    1, /* m */    0, /* n */    0, /* o */    0, /* p */
     0, /* q */   -1, /* r */    0, /* s */   -1, /* t */    0, /* u */
     0, /* v */    1, /* w */    0, /* x */    0, /* y */    0, /* z */
    -1, /* { */   -3, /* | */   -1, /* } */    0  /* ~ */
};

const font_t FONT6X6 = {
    FONT6X6_WIDTH,
    FONT6X6_HEIGHT,

    FONT6X6_SPACE_WIDTH,
    FONT6X6_SPACING,
    FONT6X6_LINE_HEIGHT,

    FONT6X6_SPACINGS
};

const uint8_t FONT6X6_DATA[] = {
    3,0,0,0,0,
    3,0,0,0,0,
    2,0,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    3,0,3,0,0,
    2,0,2,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,2,1,3,0,
    2,3,2,3,2,
    0,2,1,3,0,
    2,3,2,3,2,
    0,3,1,3,0,
    0,0,0,0,0,
    1,2,3,2,1,
    2,1,2,1,0,
    1,2,3,2,1,
    0,1,2,1,2,
    1,2,3,2,1,
    0,0,2,0,0,
    2,2,0,1,2,
    2,3,1,3,1,
    0,1,3,1,0,
    1,3,1,3,2,
    2,1,0,2,2,
    0,0,0,0,0,
    1,3,2,1,0,
    2,1,1,3,0,
    1,3,3,1,0,
    2,1,2,3,0,
    1,2,3,3,0,
    0,0,0,0,0,
    0,3,0,0,0,
    1,2,0,0,0,
    2,1,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,2,3,0,0,
    2,1,0,0,0,
    3,0,0,0,0,
    2,1,0,0,0,
    1,2,3,0,0,
    0,0,0,0,0,
    3,2,1,0,0,
    0,1,2,0,0,
    0,0,3,0,0,
    0,1,2,0,0,
    3,2,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    2,1,2,0,0,
    1,3,1,0,0,
    2,1,2,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,3,1,0,0,
    3,3,2,0,0,
    1,2,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,0,0,0,0,
    3,0,0,0,0,
    2,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,2,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    0,0,3,0,0,
    0,1,2,0,0,
    1,3,1,0,0,
    2,1,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,3,0,
    3,1,2,3,0,
    3,2,1,2,0,
    1,3,2,1,0,
    0,0,0,0,0,
    1,2,0,0,0,
    2,3,0,0,0,
    0,3,0,0,0,
    0,3,0,0,0,
    0,3,0,0,0,
    0,0,0,0,0,
    2,3,3,1,0,
    0,0,1,2,0,
    1,3,3,1,0,
    2,1,0,0,0,
    3,3,3,2,0,
    0,0,0,0,0,
    2,3,3,1,0,
    0,0,1,2,0,
    0,2,3,1,0,
    0,0,1,2,0,
    2,3,3,1,0,
    0,0,0,0,0,
    0,1,2,0,0,
    1,2,3,0,0,
    2,1,3,0,0,
    3,3,3,2,0,
    0,0,3,0,0,
    0,0,0,0,0,
    3,3,3,2,0,
    3,1,0,0,0,
    2,3,2,1,0,
    0,0,1,2,0,
    2,3,2,1,0,
    0,0,0,0,0,
    1,2,3,2,0,
    2,1,0,0,0,
    3,3,2,1,0,
    2,1,1,3,0,
    1,2,3,1,0,
    0,0,0,0,0,
    2,3,3,3,0,
    0,0,1,2,0,
    0,1,3,1,0,
    1,3,1,0,0,
    2,1,0,0,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,2,0,
    1,3,3,1,0,
    2,1,1,2,0,
    1,3,3,1,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,2,0,
    1,2,3,3,0,
    0,0,0,2,0,
    2,3,2,1,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    2,0,0,0,0,
    0,1,2,0,0,
    1,3,1,0,0,
    3,1,0,0,0,
    1,3,1,0,0,
    0,1,2,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    2,3,3,2,0,
    0,0,0,0,0,
    2,3,3,2,0,
    0,0,0,0,0,
    0,0,0,0,0,
    2,1,0,0,0,
    1,3,1,0,0,
    0,1,2,0,0,
    1,3,1,0,0,
    2,1,0,0,0,
    0,0,0,0,0,
    1,3,2,1,0,
    2,1,1,2,0,
    0,0,3,1,0,
    0,0,1,0,0,
    0,0,3,0,0,
    0,0,0,0,0,
    1,2,3,2,1,
    2,3,3,3,2,
    3,2,3,3,2,
    2,1,0,0,0,
    1,2,3,3,1,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,2,0,
    2,3,3,3,0,
    3,1,1,3,0,
    3,0,0,3,0,
    0,0,0,0,0,
    2,3,2,1,0,
    3,1,1,2,0,
    3,3,2,1,0,
    3,1,1,2,0,
    3,3,2,1,0,
    0,0,0,0,0,
    1,3,3,0,0,
    2,1,1,2,0,
    3,0,0,0,0,
    2,1,1,2,0,
    1,3,3,0,0,
    0,0,0,0,0,
    3,3,2,1,0,
    3,1,1,2,0,
    3,0,0,3,0,
    3,1,1,2,0,
    3,3,2,1,0,
    0,0,0,0,0,
    2,3,3,2,0,
    3,1,0,0,0,
    3,3,2,0,0,
    3,1,0,0,0,
    3,3,3,2,0,
    0,0,0,0,0,
    3,3,3,2,0,
    3,1,0,0,0,
    3,3,2,0,0,
    3,1,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    1,3,2,1,0,
    2,1,1,2,0,
    3,0,0,0,0,
    3,1,3,3,0,
    1,3,3,2,0,
    0,0,0,0,0,
    2,0,0,3,0,
    3,1,1,3,0,
    3,3,3,3,0,
    3,1,1,3,0,
    2,0,0,2,0,
    0,0,0,0,0,
    2,3,2,0,0,
    1,3,1,0,0,
    0,3,0,0,0,
    1,3,1,0,0,
    2,3,2,0,0,
    0,0,0,0,0,
    0,0,0,2,0,
    0,0,0,3,0,
    0,0,0,3,0,
    2,1,1,2,0,
    1,2,3,1,0,
    0,0,0,0,0,
    3,0,1,2,0,
    3,1,3,1,0,
    3,3,1,0,0,
    3,1,3,1,0,
    2,0,1,2,0,
    0,0,0,0,0,
    2,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    3,1,0,0,0,
    3,3,3,2,0,
    0,0,0,0,0,
    3,1,0,1,3,
    3,2,1,2,3,
    3,1,3,1,3,
    3,0,1,0,3,
    2,0,0,0,2,
    0,0,0,0,0,
    2,1,0,2,0,
    3,3,1,3,0,
    3,1,2,3,0,
    3,0,1,3,0,
    3,0,0,3,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,2,0,
    3,0,0,3,0,
    2,1,1,2,0,
    1,3,3,1,0,
    0,0,0,0,0,
    3,3,2,1,0,
    3,1,1,2,0,
    3,1,1,2,0,
    3,3,2,1,0,
    3,1,0,0,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,2,0,
    3,0,0,3,0,
    2,1,2,3,0,
    1,2,3,2,0,
    0,0,0,0,0,
    3,3,2,1,0,
    3,1,1,2,0,
    3,1,1,2,0,
    3,3,3,1,0,
    3,1,1,2,0,
    0,0,0,0,0,
    1,2,3,2,0,
    2,1,0,0,0,
    1,3,2,1,0,
    0,0,1,2,0,
    2,3,2,1,0,
    0,0,0,0,0,
    2,3,2,0,0,
    1,3,1,0,0,
    0,3,0,0,0,
    0,3,0,0,0,
    0,3,0,0,0,
    0,0,0,0,0,
    2,0,0,3,0,
    3,0,0,3,0,
    3,0,0,3,0,
    2,1,1,2,0,
    1,3,2,1,0,
    0,0,0,0,0,
    3,0,0,3,0,
    3,0,1,2,0,
    2,1,3,1,0,
    2,1,2,0,0,
    1,2,1,0,0,
    0,0,0,0,0,
    2,0,0,0,3,
    3,0,0,0,3,
    3,1,2,1,3,
    3,3,1,3,3,
    2,1,0,1,2,
    0,0,0,0,0,
    3,0,0,3,0,
    2,1,1,2,0,
    1,2,2,1,0,
    2,1,1,2,0,
    3,0,0,3,0,
    0,0,0,0,0,
    3,0,0,3,0,
    2,1,1,2,0,
    1,2,2,1,0,
    0,3,1,0,0,
    0,3,0,0,0,
    0,0,0,0,0,
    2,3,3,3,0,
    0,0,1,2,0,
    1,2,2,1,0,
    2,1,0,0,0,
    3,3,3,2,0,
    0,0,0,0,0,
    3,2,0,0,0,
    3,1,0,0,0,
    3,0,0,0,0,
    3,1,0,0,0,
    3,2,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    2,1,0,0,0,
    1,3,1,0,0,
    0,1,2,0,0,
    0,0,3,0,0,
    0,0,0,0,0,
    2,3,0,0,0,
    1,3,0,0,0,
    0,3,0,0,0,
    1,3,0,0,0,
    2,3,0,0,0,
    0,0,0,0,0,
    1,3,1,0,0,
    2,1,2,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,3,2,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,3,0,
    3,1,1,3,0,
    1,3,2,3,0,
    0,0,0,0,0,
    3,1,0,0,0,
    3,2,3,1,0,
    3,0,1,3,0,
    3,0,1,2,0,
    3,3,2,1,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,3,3,0,0,
    2,1,0,0,0,
    3,1,1,2,0,
    1,3,3,1,0,
    0,0,0,0,0,
    0,0,1,3,0,
    1,2,3,3,0,
    2,1,0,3,0,
    2,1,0,3,0,
    1,2,3,3,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,3,3,1,0,
    2,1,2,3,0,
    3,2,1,0,0,
    1,3,3,2,0,
    0,0,0,0,0,
    1,2,3,0,0,
    2,1,0,0,0,
    3,3,2,0,0,
    3,1,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,3,2,1,0,
    2,1,1,2,0,
    1,3,3,3,0,
    0,0,1,2,0,
    0,2,3,1,0,
    3,0,0,0,0,
    3,2,2,1,0,
    3,0,1,2,0,
    3,0,0,3,0,
    2,0,0,3,0,
    0,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    2,0,0,0,0,
    0,0,0,0,0,
    0,0,3,0,0,
    0,0,0,0,0,
    0,0,2,0,0,
    0,0,3,0,0,
    0,1,2,0,0,
    2,3,1,0,0,
    3,0,0,0,0,
    3,0,1,2,0,
    3,1,2,1,0,
    3,3,2,1,0,
    3,0,1,2,0,
    0,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    2,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    2,3,1,2,1,
    3,1,3,1,2,
    3,0,3,0,3,
    3,0,3,0,3,
    0,0,0,0,0,
    0,0,0,0,0,
    3,3,2,1,0,
    3,0,1,2,0,
    3,0,0,3,0,
    3,0,0,3,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,3,0,
    3,1,1,3,0,
    1,3,3,1,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,3,2,1,0,
    3,0,1,3,0,
    3,0,1,3,0,
    3,3,2,1,0,
    3,0,0,0,0,
    0,0,0,0,0,
    1,2,3,1,0,
    2,1,1,3,0,
    3,1,0,3,0,
    1,2,3,3,0,
    0,0,0,3,0,
    0,0,0,0,0,
    1,2,2,0,0,
    2,1,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,2,3,2,0,
    2,1,0,0,0,
    1,3,3,2,0,
    2,3,3,1,0,
    0,0,0,0,0,
    1,3,1,0,0,
    2,3,2,0,0,
    1,3,1,0,0,
    0,3,0,0,0,
    0,2,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    2,0,0,2,0,
    3,0,0,3,0,
    2,1,1,3,0,
    1,2,3,3,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,0,0,3,0,
    3,0,1,2,0,
    2,1,2,1,0,
    1,2,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    2,0,0,0,3,
    3,0,0,0,3,
    2,1,2,1,2,
    1,3,1,3,1,
    0,0,0,0,0,
    0,0,0,0,0,
    2,1,1,3,0,
    1,2,3,1,0,
    1,3,2,1,0,
    3,1,1,3,0,
    0,0,0,0,0,
    0,0,0,0,0,
    3,0,0,3,0,
    2,1,0,2,0,
    1,2,1,2,0,
    0,1,2,1,0,
    0,3,1,0,0,
    0,0,0,0,0,
    2,3,3,2,0,
    0,1,2,0,0,
    1,2,1,0,0,
    2,3,3,2,0,
    0,0,0,0,0,
    0,2,3,0,0,
    1,3,1,0,0,
    2,1,0,0,0,
    1,3,1,0,0,
    0,2,3,0,0,
    0,0,0,0,0,
    2,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    3,0,0,0,0,
    2,0,0,0,0,
    3,2,0,0,0,
    1,3,1,0,0,
    0,1,2,0,0,
    1,3,1,0,0,
    3,2,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,3,1,2,0,
    2,1,3,1,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0
};

#endif
