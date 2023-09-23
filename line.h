#ifndef LINE_H
#define LINE_H

static inline void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c) {
    uint8_t steep = 0;
    int16_t dx, dy, error2, derror2;

    if (abs16(x0 - x1) < abs16(y0 - y1)) {
        swap16(x0, y0);
        swap16(x1, y1);
        steep = 1;
    }

    if (x0 > x1) {
        swap16(x0, x1);
        swap16(y0, y1);
    }

    dx = x1 - x0;
    dy = y1 - y0;
    derror2 = abs16(dy) << 1;
    error2 = 0;

    if (steep) {
        int32_t x, x0i, x1i;
        int16_t y = y0;

        if (y1 > y0) {
            if (y >= SCREEN_WIDTH)
                return;

            x0i = mul_by_screen_stride32(x0);
            x1i = mul_by_screen_stride32(x1);

            for (x = x0i; x <= x1i; x += SCREEN_WIDTH) {
                if (x >= 0 && y >= 0 && x < (int32_t)SCREEN_WIDTH * SCREEN_HEIGHT)
                    dblbuf[y + x] = c;

                error2 += derror2;
                if (error2 > dx) {
                    if (y == SCREEN_WIDTH - 1)
                        return;

                    y++;
                    aw_assert(y < SCREEN_WIDTH);
                    error2 -= dx << 1;
                }
            }
        } else {
            if (y < 0)
                return;

            x0i = mul_by_screen_stride32(x0);
            x1i = mul_by_screen_stride32(x1);

            for (x = x0i; x <= x1i; x += SCREEN_WIDTH) {
                if (x >= 0 && y < SCREEN_WIDTH && x < (int32_t)SCREEN_WIDTH * SCREEN_HEIGHT)
                    dblbuf[y + x] = c;

                error2 += derror2;
                if (error2 > dx) {
                    if (y == 0)
                        return;

                    y--;
                    aw_assert(y >= 0);
                    error2 -= dx << 1;
                }
            }
        }
    } else {
        int16_t x;
        int32_t y;

        if (y1 > y0) {
            if (y0 >= SCREEN_HEIGHT)
                return;

            y = mul_by_screen_stride32(y0);

            for (x = x0; x <= x1; x++) {
                if (x >= 0 && y >= 0 && x < SCREEN_WIDTH)
                    dblbuf[x + y] = c;

                error2 += derror2;
                if (error2 > dx) {
                    if (y == (int32_t)SCREEN_WIDTH * (SCREEN_HEIGHT - 1))
                        return;

                    y += SCREEN_WIDTH;
                    aw_assert(y < (int32_t)SCREEN_WIDTH * SCREEN_HEIGHT);
                    error2 -= dx << 1;
                }
            }
        } else {
            if (y0 < 0)
                return;

            y = mul_by_screen_stride32(y0);

            for (x = x0; x <= x1; x++) {
                if (x >= 0 && x < SCREEN_WIDTH && y < (int32_t)SCREEN_WIDTH * SCREEN_HEIGHT)
                    dblbuf[x + y] = c;

                error2 += derror2;
                if (error2 > dx) {
                    if (y == 0)
                        return;

                    y -= SCREEN_WIDTH;
                    aw_assert(y >= 0);
                    error2 -= dx << 1;
                }
            }
        }
    }
}

#endif
