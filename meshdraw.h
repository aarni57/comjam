#include "draw.h"
#include "ship.h"
#include "asteroid.h"
#include "scrap.h"
#include "scrap2.h"
#include "scrap3.h"

static fx4x3_t ship_mesh_adjust_matrix;
static fx4x3_t asteroid_mesh_adjust_matrix;
static fx4x3_t scrap_mesh_adjust_matrix;
static fx4x3_t scrap2_mesh_adjust_matrix;
static fx4x3_t scrap3_mesh_adjust_matrix;

static void init_mesh_adjustment_matrix(fx4x3_t* m, const fx3_t* size, const fx3_t* center) {
    fx4x3_identity(m);
    m->m[FX4X3_00] = size->x << 4;
    m->m[FX4X3_11] = size->y << 4;
    m->m[FX4X3_22] = size->z << 4;
    m->m[FX4X3_30] = center->x >> 4;
    m->m[FX4X3_31] = center->y >> 4;
    m->m[FX4X3_32] = center->z >> 4;
}

static void init_meshes() {
    init_mesh_adjustment_matrix(&ship_mesh_adjust_matrix, &ship_size, &ship_center);
    init_mesh_adjustment_matrix(&asteroid_mesh_adjust_matrix, &asteroid_size, &asteroid_center);
    init_mesh_adjustment_matrix(&scrap_mesh_adjust_matrix, &scrap_size, &scrap_center);
    init_mesh_adjustment_matrix(&scrap2_mesh_adjust_matrix, &scrap2_size, &scrap2_center);
    init_mesh_adjustment_matrix(&scrap3_mesh_adjust_matrix, &scrap3_size, &scrap3_center);
}

static void draw_ship(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &ship_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        ship_num_indices, ship_num_vertices,
        ship_indices, ship_face_colors, ship_vertices);
}

static void draw_asteroid(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &asteroid_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        asteroid_num_indices, asteroid_num_vertices,
        asteroid_indices, asteroid_face_colors, asteroid_vertices);
}

static void draw_scrap(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &scrap_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        scrap_num_indices, scrap_num_vertices,
        scrap_indices, scrap_face_colors, scrap_vertices);
}

static void draw_scrap2(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &scrap2_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        scrap2_num_indices, scrap2_num_vertices,
        scrap2_indices, scrap2_face_colors, scrap2_vertices);
}

static void draw_scrap3(const fx4x3_t* view_matrix, const fx4_t* rotation,
    const fx3_t* translation) {
    fx4x3_t model_matrix, tmp, model_view_matrix;

    fx4x3_rotation_translation(&model_matrix, rotation, translation);

    fx4x3_mul(&tmp, &model_matrix, &scrap3_mesh_adjust_matrix);
    fx4x3_mul(&model_view_matrix, view_matrix, &tmp);

    draw_mesh(&model_view_matrix,
        scrap3_num_indices, scrap3_num_vertices,
        scrap3_indices, scrap3_face_colors, scrap3_vertices);
}
