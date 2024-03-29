#include "fxtypes.h"

#define scrap_num_vertices 28
#define scrap_num_indices 156

static const fx3_t scrap_center = { 237, 234, -236 };
static const fx3_t scrap_size = { 5523, 1236, 5524 };

static const uint16_t scrap_indices[] = {
0, 1, 2, 2, 4, 0, 6, 7, 8, 8, 3, 6, 12, 1, 0, 0,
13, 12, 18, 10, 7, 7, 6, 18, 2, 5, 23, 23, 4, 2, 8, 12,
13, 13, 3, 8, 22, 27, 15, 15, 17, 22, 24, 26, 21, 21, 25, 24,
22, 17, 11, 11, 9, 22, 20, 24, 25, 25, 19, 20, 16, 14, 15, 15,
27, 16, 9, 11, 21, 21, 26, 9, 18, 6, 24, 24, 20, 18, 2, 1,
17, 17, 15, 2, 4, 23, 16, 16, 27, 4, 23, 5, 14, 14, 16, 23,
1, 12, 11, 11, 17, 1, 0, 4, 27, 27, 22, 0, 12, 8, 21, 21,
11, 12, 13, 0, 22, 22, 9, 13, 10, 18, 20, 20, 19, 10, 3, 13,
9, 9, 26, 3, 8, 7, 25, 25, 21, 8, 7, 10, 19, 19, 25, 7,
6, 3, 26, 26, 24, 6, 5, 2, 15, 15, 14, 5, };

static const uint8_t scrap_face_colors[] = {
40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 70, 70, 70, 70,
70, 70, 70, 70, 70, 70, 70, 70, 121, 121, 121, 121, 40, 40, 121, 121,
69, 69, 40, 40, 67, 67, 40, 40, 40, 40, 67, 67, 40, 40, 40, 40,
70, 70, 70, 70, };

static const int8_t scrap_vertices[] = {
-128, -128, 0,
-90, -128, 0,
-64, -128, -64,
0, -128, 127,
-91, -128, -91,
0, -128, -90,
90, -128, 90,
63, -128, 63,
0, -128, 89,
-91, 127, 90,
89, -128, 0,
-64, 127, 63,
-64, -128, 63,
-91, -128, 90,
0, 127, -90,
-64, 127, -64,
0, 127, -128,
-90, 127, 0,
127, -128, 0,
89, 127, 0,
127, 127, 0,
0, 127, 89,
-128, 127, 0,
0, -128, -128,
90, 127, 90,
63, 127, 63,
0, 127, 127,
-91, 127, -91,
};
