#include "util.h"

kernel void mult(global const float *u, global const float *v, global float *w, int const a, int const b, int const c) {
    size_t i = get_local_id(0);
    size_t j = get_local_id(1);
    size_t global_i = get_global_id(0);
    size_t global_j = get_global_id(1);

    local float local_a[TILE_W][TILE_H]; // @todo: find out what is what if different
    local float local_b[TILE_W][TILE_H];

    float tmp_sum = 0;

    for (int t = 0; t < b; t += TILE_W) {
        local_a[i][j] = u[global_i * b + global_j];
        local_b[j][i] = v[global_j * c + global_i];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (int d = 0; d < TILE_H; ++d) {
            tmp_sum += local_a[i][d] * local_b[d][j];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    w[global_i * a + global_j] = tmp_sum;
}