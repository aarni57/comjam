#define NUM_DEBRIS 56
static fx3_t debris_positions[NUM_DEBRIS];
static fx3_t debris_rotation_axes[NUM_DEBRIS];
static fx_t debris_rotations[NUM_DEBRIS];
static fx_t debris_rotation_speeds[NUM_DEBRIS];
static fx3_t debris_speeds[NUM_DEBRIS];

static inline void randomize_debris_properties(uint16_t i) {
    fx3_t r;
    r.x = fx_random_signed_one();
    r.y = fx_random_signed_one();
    r.z = fx_random_signed_one();
    fx3_normalize_ip(&r);
    debris_rotation_axes[i] = r;

    debris_rotations[i] = fx_random_one();

    switch (i & 3) {
        case 0:
        case 1:
        case 2:
            debris_rotation_speeds[i] = (fx_random_one() >> 7) + 256;
            debris_speeds[i].x = fx_random_one() >> 12;
            debris_speeds[i].y = fx_random_one() >> 12;
            debris_speeds[i].z = fx_random_one() >> 12;
            break;
        case 3:
            debris_rotation_speeds[i] = (fx_random_one() >> 8) + 256;
            debris_speeds[i].x = fx_random_one() >> 13;
            debris_speeds[i].y = fx_random_one() >> 13;
            debris_speeds[i].z = fx_random_one() >> 13;
            break;
    }
}

static void init_debris() {
    uint16_t i;
    for (i = 0; i < NUM_DEBRIS; ++i) {
        fx3_t r;
        r.x = random_debris_x();
        r.y = random_debris_x();
        r.z = random_debris_x();
        debris_positions[i] = r;
        randomize_debris_properties(i);
    }
}

#define DEBRIS_BOX_SIZE 65536L
#define DEBRIS_BOX_HALF_SIZE (DEBRIS_BOX_SIZE >> 1)

static void update_debris(const fx3_t* center, const fx3_t* movement) {
    uint16_t i;
    fx_t x_low, x_high, y_low, y_high, z_low, z_high;

    x_low = center->x - DEBRIS_BOX_HALF_SIZE;
    x_high = center->x + DEBRIS_BOX_HALF_SIZE;
    y_low = center->y - DEBRIS_BOX_HALF_SIZE;
    y_high = center->y + DEBRIS_BOX_HALF_SIZE;
    z_low = center->z - DEBRIS_BOX_HALF_SIZE;
    z_high = center->z + DEBRIS_BOX_HALF_SIZE;

    for (i = 0; i < NUM_DEBRIS; ++i) {
        fx3_t* p = &debris_positions[i];
        fx3_add_ip(p, &debris_speeds[i]);
        fx3_add_ip(p, movement);

        if (p->x <= x_low) {
            p->x += DEBRIS_BOX_SIZE;
            p->y = random_debris_x();
            p->z = random_debris_x();
        } else if (p->x >= x_high) {
            p->x -= DEBRIS_BOX_SIZE;
            p->y = random_debris_x();
            p->z = random_debris_x();
        } else if (p->y <= y_low) {
            p->y += DEBRIS_BOX_SIZE;
            p->x = random_debris_x();
            p->z = random_debris_x();
        } else if (p->y >= y_high) {
            p->y -= DEBRIS_BOX_SIZE;
            p->x = random_debris_x();
            p->z = random_debris_x();
        } else if (p->z <= z_low) {
            p->z += DEBRIS_BOX_SIZE;
            p->x = random_debris_x();
            p->y = random_debris_x();
        } else if (p->z >= z_high) {
            p->z -= DEBRIS_BOX_SIZE;
            p->x = random_debris_x();
            p->y = random_debris_x();
        }

        debris_rotations[i] += debris_rotation_speeds[i];
        if (debris_rotations[i] >= FX_ONE) {
            debris_rotations[i] -= FX_ONE;
        }
    }
}

#define DEBRIS_CLIP_BORDER 32
#define DEBRIS_LEFT_CLIP (-DEBRIS_CLIP_BORDER << RASTER_SUBPIXEL_BITS)
#define DEBRIS_RIGHT_CLIP ((SCREEN_WIDTH + DEBRIS_CLIP_BORDER) << RASTER_SUBPIXEL_BITS)
#define DEBRIS_TOP_CLIP (-DEBRIS_CLIP_BORDER << RASTER_SUBPIXEL_BITS)
#define DEBRIS_BOTTOM_CLIP ((SCREEN_HEIGHT + DEBRIS_CLIP_BORDER) << RASTER_SUBPIXEL_BITS)

static void draw_debris(const fx4x3_t* view_matrix) {
    fx4_t rotation;
    fx_t rotation_angle;
    fx3_t transformed_position;
    const fx3_t* position_iter = debris_positions;
    const fx_t* rotation_iter = debris_rotations;
    uint16_t i;
    for (i = 0; i < NUM_DEBRIS; ++i) {
        const fx3_t* position = position_iter++;
        rotation_angle = *rotation_iter++;

        fx_transform_point(&transformed_position, view_matrix, position);
        if (transformed_position.z < NEAR_CLIP)
            continue;

        project_to_screen(&transformed_position);
        if (transformed_position.x < DEBRIS_LEFT_CLIP ||
            transformed_position.x > DEBRIS_RIGHT_CLIP ||
            transformed_position.y < DEBRIS_TOP_CLIP ||
            transformed_position.y > DEBRIS_BOTTOM_CLIP)
            continue;

        fx_quat_rotation_axis_angle(&rotation, &debris_rotation_axes[i], rotation_angle);

        switch (i & 3) {
            case 0:
                draw_scrap(view_matrix, &rotation, position);
                break;
            case 1:
                draw_scrap2(view_matrix, &rotation, position);
                break;
            case 2:
                draw_scrap3(view_matrix, &rotation, position);
                break;
            case 3:
                draw_asteroid(view_matrix, &rotation, position);
                break;
        }
    }
}
