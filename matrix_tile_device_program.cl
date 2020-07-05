#include "util.h"


// requires TILE_H == TILE_W
kernel void mult(global const float *a, global const float *b, global float *c, int const M, int const K, int const N) {
    size_t i = get_local_id(0);
    size_t j = get_local_id(1);
    size_t global_i = get_global_id(0);
    size_t global_j = get_global_id(1);

    local float local_a[TILE_H][TILE_W];
    local float local_b[TILE_W][TILE_H];

    float tmp_sum = 0.0;

    for (int t = 0; t < K; t += TILE_W) {
        local_a[i][j] = a[global_i * K + t + j];
        local_b[j][i] = b[(t + i) * N + global_j];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (int d = 0; d < TILE_W; ++d) {
            tmp_sum += local_a[i][d] * local_b[j][d];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    c[global_i * N + global_j] = tmp_sum;
}