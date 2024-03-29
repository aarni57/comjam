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

static const int8_t FONT6X6_SPACINGS[] = {
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

static const font_t FONT6X6 = {
    FONT6X6_WIDTH,
    FONT6X6_HEIGHT,

    FONT6X6_SPACE_WIDTH,
    FONT6X6_SPACING,
    FONT6X6_LINE_HEIGHT,

    FONT6X6_SPACINGS
};

static const uint8_t FONT6X6_DATA[] = {
    3, 12, 32, 0, 0, 3, 0, 48, 131, 8,
    0, 0, 0, 0, 0, 216, 184, 139, 141, 187,
    220, 0, 144, 155, 25, 185, 145, 153, 27, 8,
    74, 122, 71, 71, 183, 134, 2, 208, 134, 53,
    125, 152, 147, 15, 0, 12, 36, 96, 0, 0,
    0, 0, 144, 131, 1, 3, 24, 144, 3, 0,
    27, 144, 0, 3, 9, 27, 0, 0, 128, 9,
    29, 152, 0, 0, 0, 0, 116, 240, 66, 6,
    0, 0, 0, 0, 0, 0, 4, 48, 128, 0,
    0, 0, 176, 0, 0, 0, 0, 0, 0, 0,
    0, 4, 48, 0, 0, 48, 144, 208, 129, 1,
    3, 0, 144, 135, 53, 231, 108, 210, 6, 0,
    9, 56, 192, 0, 3, 12, 0, 224, 7, 36,
    125, 24, 240, 11, 0, 126, 64, 130, 7, 36,
    126, 0, 64, 66, 14, 54, 252, 2, 3, 0,
    191, 28, 224, 6, 36, 110, 0, 144, 139, 1,
    111, 88, 147, 7, 0, 254, 64, 66, 71, 7,
    6, 0, 144, 135, 37, 125, 88, 210, 7, 0,
    121, 88, 146, 15, 32, 110, 0, 0, 192, 0,
    0, 12, 0, 0, 0, 0, 0, 48, 0, 0,
    3, 8, 64, 66, 7, 7, 116, 64, 2, 0,
    0, 248, 2, 128, 47, 0, 0, 96, 64, 7,
    36, 116, 96, 0, 0, 109, 88, 2, 7, 4,
    48, 0, 144, 155, 191, 251, 26, 144, 31, 0,
    121, 88, 226, 207, 53, 195, 0, 224, 198, 37,
    111, 92, 242, 6, 0, 61, 88, 50, 128, 37,
    61, 0, 240, 198, 37, 195, 92, 242, 6, 0,
    190, 28, 240, 194, 1, 191, 0, 240, 203, 1,
    47, 28, 48, 0, 0, 109, 88, 50, 192, 61,
    189, 0, 32, 204, 53, 255, 92, 35, 8, 0,
    46, 116, 192, 64, 7, 46, 0, 0, 8, 48,
    192, 88, 146, 7, 0, 147, 220, 241, 193, 29,
    146, 0, 32, 192, 0, 3, 28, 240, 11, 0,
    71, 111, 126, 247, 196, 2, 2, 96, 200, 55,
    231, 76, 51, 12, 0, 121, 88, 50, 140, 37,
    125, 0, 240, 198, 37, 151, 188, 113, 0, 0,
    121, 88, 50, 140, 57, 185, 0, 240, 198, 37,
    151, 252, 113, 9, 0, 185, 24, 208, 6, 36,
    110, 0, 224, 66, 7, 12, 48, 192, 0, 0,
    194, 12, 51, 140, 37, 109, 0, 48, 204, 36,
    118, 152, 144, 1, 0, 2, 15, 124, 246, 247,
    70, 2, 48, 140, 37, 105, 88, 50, 12, 0,
    195, 88, 146, 6, 7, 12, 0, 224, 15, 36,
    105, 24, 240, 11, 0, 11, 28, 48, 192, 1,
    11, 0, 48, 128, 1, 29, 144, 0, 3, 0,
    14, 52, 192, 64, 3, 14, 0, 208, 129, 9,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    47, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 228, 97, 205, 53, 237, 0, 112, 192, 30,
    211, 76, 242, 6, 0, 0, 244, 96, 192, 37,
    125, 0, 0, 77, 62, 198, 24, 147, 15, 0,
    0, 244, 97, 206, 6, 189, 0, 144, 131, 1,
    47, 28, 48, 0, 0, 0, 180, 97, 73, 63,
    144, 224, 49, 192, 26, 147, 12, 35, 12, 0,
    3, 0, 48, 192, 0, 2, 0, 0, 3, 0,
    32, 192, 64, 130, 7, 3, 76, 114, 198, 27,
    147, 0, 48, 192, 0, 3, 12, 32, 0, 0,
    0, 120, 118, 231, 204, 51, 3, 0, 192, 27,
    147, 12, 51, 12, 0, 0, 228, 97, 205, 53,
    125, 0, 0, 192, 27, 211, 76, 243, 198, 0,
    0, 228, 97, 205, 49, 249, 0, 3, 64, 10,
    6, 12, 48, 0, 0, 0, 228, 98, 64, 47,
    126, 0, 208, 129, 11, 29, 48, 128, 0, 0,
    0, 8, 50, 140, 53, 249, 0, 0, 192, 48,
    147, 152, 145, 1, 0, 0, 8, 60, 176, 153,
    221, 1, 0, 128, 53, 121, 180, 113, 13, 0,
    0, 12, 99, 72, 38, 100, 112, 0, 128, 47,
    36, 100, 224, 11, 0, 56, 116, 96, 64, 7,
    56, 0, 32, 192, 0, 3, 12, 48, 128, 0,
    11, 116, 64, 66, 7, 11, 0, 0, 64, 39,
    118, 0, 0, 0, 0,
};

#endif
