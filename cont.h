#include "fxtypes.h"

#define container_num_vertices 74
#define container_num_indices 432

static const fx3_t container_center = { 0, 0, 0 };
static const fx3_t container_size = { 18680, 19244, 18906 };

static const uint16_t container_indices[] = {
0, 1, 2, 0, 2, 5, 0, 5, 8, 9, 10, 11, 11, 13, 9, 15,
16, 17, 15, 17, 20, 15, 20, 23, 11, 20, 17, 5, 13, 8, 30, 31,
32, 30, 32, 35, 30, 35, 38, 30, 38, 41, 9, 43, 38, 38, 35, 9,
11, 10, 50, 50, 20, 11, 2, 55, 43, 43, 5, 2, 23, 20, 50, 50,
64, 23, 13, 5, 43, 43, 9, 13, 9, 35, 32, 32, 10, 9, 55, 41,
38, 38, 43, 55, 64, 50, 31, 31, 30, 64, 10, 32, 31, 31, 50, 10,
13, 11, 17, 13, 17, 16, 13, 16, 0, 13, 0, 8, 6, 71, 62, 6,
62, 2, 6, 2, 1, 65, 57, 61, 61, 51, 65, 15, 23, 73, 15, 73,
72, 15, 72, 56, 65, 72, 73, 62, 71, 51, 30, 41, 69, 30, 69, 70,
30, 70, 66, 30, 66, 63, 61, 70, 69, 69, 53, 61, 68, 57, 65, 65,
73, 68, 53, 55, 2, 2, 62, 53, 23, 64, 68, 68, 73, 23, 51, 61,
53, 53, 62, 51, 66, 70, 61, 61, 57, 66, 69, 41, 55, 55, 53, 69,
64, 30, 63, 63, 68, 64, 63, 66, 57, 57, 68, 63, 51, 71, 6, 51,
6, 56, 51, 56, 72, 51, 72, 65, 0, 37, 46, 0, 46, 22, 0, 22,
1, 42, 58, 40, 40, 44, 42, 15, 19, 39, 15, 39, 24, 15, 24, 16,
42, 24, 39, 46, 37, 44, 21, 67, 60, 21, 60, 52, 21, 52, 45, 21,
45, 34, 40, 52, 60, 60, 33, 40, 36, 58, 42, 42, 39, 36, 33, 49,
22, 22, 46, 33, 19, 29, 36, 36, 39, 19, 44, 40, 33, 33, 46, 44,
45, 52, 40, 40, 58, 45, 60, 67, 49, 49, 33, 60, 29, 21, 34, 34,
36, 29, 34, 45, 58, 58, 36, 34, 44, 37, 0, 44, 0, 16, 44, 16,
24, 44, 24, 42, 6, 1, 22, 6, 22, 28, 6, 28, 3, 26, 4, 59,
59, 25, 26, 15, 56, 47, 15, 47, 12, 15, 12, 19, 59, 12, 47, 28,
25, 3, 21, 18, 54, 21, 54, 48, 21, 48, 27, 21, 27, 67, 26, 7,
27, 27, 48, 26, 59, 4, 14, 14, 12, 59, 22, 49, 7, 7, 28, 22,
19, 12, 14, 14, 29, 19, 25, 28, 7, 7, 26, 25, 26, 48, 54, 54,
4, 26, 49, 67, 27, 27, 7, 49, 29, 14, 18, 18, 21, 29, 4, 54,
18, 18, 14, 4, 25, 59, 47, 25, 47, 56, 25, 56, 6, 25, 6, 3,
};

static const uint8_t container_face_colors[] = {
1, 1, 1, 74, 74, 75, 75, 75, 74, 1, 70, 70, 70, 70, 104, 104,
75, 75, 75, 75, 67, 75, 75, 75, 121, 121, 67, 67, 1, 1, 67, 67,
85, 85, 85, 85, 1, 1, 1, 104, 104, 75, 75, 75, 104, 67, 70, 70,
70, 70, 74, 74, 67, 67, 75, 75, 75, 67, 67, 67, 40, 40, 67, 67,
1, 1, 1, 1, 104, 104, 104, 104, 1, 1, 1, 93, 93, 75, 75, 75,
1, 85, 67, 67, 67, 67, 75, 67, 1, 1, 1, 1, 1, 1, 93, 93,
59, 59, 74, 74, 93, 93, 74, 74, 85, 85, 85, 85, 1, 1, 1, 74,
74, 75, 75, 75, 67, 74, 67, 67, 67, 67, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 57, 57, 74, 74, 93, 93, 85, 85, 104, 104, 104, 104,
};

static const int8_t container_vertices[] = {
127, 0, 127,
0, 0, 127,
0, -128, 127,
-128, 85, 127,
-116, 127, -63,
87, -123, 127,
-128, 0, 127,
-75, 127, 108,
127, -86, 127,
115, -128, 62,
115, -128, -63,
127, -123, -87,
-88, 122, -128,
127, -123, 86,
-75, 127, -109,
0, 0, -128,
127, 0, -128,
127, -86, -128,
-63, 96, -84,
0, 127, -128,
87, -123, -128,
0, 96, -84,
0, 127, 127,
0, -128, -128,
127, 85, -128,
-128, 122, 86,
-116, 127, 62,
-63, 96, 83,
-88, 122, 127,
0, 127, -109,
0, -97, -84,
62, -97, -84,
88, -97, -49,
74, 127, 108,
62, 96, -84,
88, -97, 48,
74, 127, -109,
127, 85, 127,
62, -97, 83,
87, 122, -128,
115, 127, 62,
0, -97, 83,
127, 122, -87,
74, -128, 108,
127, 122, 86,
88, 96, -49,
87, 122, 127,
-128, 85, -128,
-89, 96, 48,
0, 127, 108,
74, -128, -109,
-128, -123, 86,
88, 96, 48,
-75, -128, 108,
-89, 96, -49,
0, -128, 108,
-128, 0, -128,
-116, -128, -63,
115, 127, -63,
-128, 122, -87,
62, 96, 83,
-116, -128, 62,
-88, -123, 127,
-63, -97, -84,
0, -128, -109,
-128, -123, -87,
-89, -97, -49,
0, 96, 83,
-75, -128, -109,
-63, -97, 83,
-89, -97, 48,
-128, -86, 127,
-128, -86, -128,
-88, -123, -128,
};
