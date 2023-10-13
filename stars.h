#define NUM_STARS 160
static int8_t star_positions[NUM_STARS * 3];

static void init_stars() {
    int8_t* p_tgt = star_positions;
    uint16_t i, j;
    for (i = 0; i < NUM_STARS; ++i) {
        int8_t x, y, z;

        for (;;) {
            uint8_t ok = 1;
            fx3_t v;
            v.x = fx_random_signed_one();
            v.y = fx_random_signed_one();
            v.z = fx_random_signed_one();
            fx3_normalize_ip(&v);

            x = clamp16(v.x >> 9, INT8_MIN, INT8_MAX);
            y = clamp16(v.y >> 9, INT8_MIN, INT8_MAX);
            z = clamp16(v.z >> 9, INT8_MIN, INT8_MAX);

            {
                int8_t* p_iter = star_positions;
                for (j = 0; j < i; ++j) {
                    int8_t u, v, w;
                    u = *p_iter++;
                    v = *p_iter++;
                    w = *p_iter++;
                    if (x == u && y == v && z == w) {
                        ok = 0;
                        break;
                    }
                }
            }

            if (ok)
                break;
        }

        *p_tgt++ = x;
        *p_tgt++ = y;
        *p_tgt++ = z;
    }
}

static void draw_stars(const fx4x3_t* view_matrix) {
    int16_t x, y;
    uint16_t i;
    fx3_t v;
    int8_t* p_iter = star_positions;

    for (i = 0; i < NUM_STARS; ++i) {
        v.x = (*p_iter++) << 9;
        v.y = (*p_iter++) << 9;
        v.z = (*p_iter++) << 9;
        fx_transform_vector_ip((const fx3x3_t*)view_matrix, &v);

        if (v.z < NEAR_CLIP)
            continue;

        project_to_screen(&v, SCREEN_SUBPIXEL_CENTER_X, SCREEN_SUBPIXEL_CENTER_Y);

        x = v.x >> RASTER_SUBPIXEL_BITS;
        y = v.y >> RASTER_SUBPIXEL_BITS;

        switch (i & 7) {
            case 0:
            case 1: {
                if (x >= 0 && x <= SCREEN_X_MAX && y >= 0 && y <= SCREEN_Y_MAX) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset] = 117;
                }

                break;
            }

            case 2:
            case 3:
            case 4: {
                if (x >= 0 && x <= SCREEN_X_MAX && y >= 0 && y <= SCREEN_Y_MAX) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset] = 9;
                }

                break;
            }

            case 5:
            case 6: {
                if (x >= 1 && x <= SCREEN_X_MAX - 1 && y >= 1 && y <= SCREEN_Y_MAX - 1) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset - SCREEN_WIDTH] = 8;
                    dblbuf[offset - 1] = 8;
                    dblbuf[offset] = 117;
                    dblbuf[offset + 1] = 8;
                    dblbuf[offset + SCREEN_WIDTH] = 8;
                }

                break;
            }

            case 7: {
                if (x >= 1 && x < SCREEN_X_MAX - 1 && y >= 1 && y < SCREEN_Y_MAX - 1) {
                    uint16_t offset = mul_by_screen_stride(y) + x;
                    dblbuf[offset - SCREEN_WIDTH] = 9;
                    dblbuf[offset - 1] = 9;
                    dblbuf[offset] = 105;
                    dblbuf[offset + 1] = 9;
                    dblbuf[offset + SCREEN_WIDTH] = 9;
                }

                break;
            }
        }
    }
}
