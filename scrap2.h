#include "fxtypes.h"

#define scrap2_num_vertices 10
#define scrap2_num_indices 48

static const fx3_t scrap2_center = { -115, 234, -197 };
static const fx3_t scrap2_size = { 4715, 1236, 5523 };

static const uint16_t scrap2_indices[] = {
0, 1, 2, 2, 4, 0, 6, 5, 3, 3, 8, 6, 7, 6, 8, 8,
9, 7, 0, 7, 9, 9, 1, 0, 5, 6, 7, 5, 7, 0, 5, 0,
4, 3, 2, 1, 3, 1, 9, 3, 9, 8, 5, 4, 2, 2, 3, 5,
};

static const uint8_t scrap2_face_colors[] = {
42, 42, 70, 70, 40, 40, 40, 40, 85, 85, 85, 70, 70, 70, 70, 70,
};

static const int8_t scrap2_vertices[] = {
-128, -128, 0,
-128, 127, 0,
21, 127, 127,
127, 127, 90,
21, -128, 127,
127, -128, 90,
21, -128, -128,
-85, -128, -91,
21, 127, -128,
-85, 127, -91,
};
