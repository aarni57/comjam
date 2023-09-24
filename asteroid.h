#include "fxtypes.h"

#define asteroid_num_vertices 8
#define asteroid_num_indices 36

const fx3_t asteroid_center = { 38, -336, 64 };
const fx3_t asteroid_size = { 6434, 6204, 6886 };

const uint16_t asteroid_indices[] = {
0, 1, 2, 1, 0, 5, 0, 2, 6, 0, 6, 5, 1, 5, 7, 2,
1, 4, 1, 7, 4, 5, 6, 7, 4, 7, 3, 2, 4, 3, 6, 2,
3, 7, 6, 3, };

const uint8_t asteroid_face_colors[] = {
69, 40, 39, 42, 40, 68, 44, 40, 44, 42, 42, 40, };

const int8_t asteroid_vertices[] = {
3, 13, -128,
127, -82, -75,
-91, -94, 6,
4, 13, 127,
47, -128, 63,
26, 127, -45,
-128, 41, -6,
95, 108, 79,
};
