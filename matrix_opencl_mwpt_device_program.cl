#include "util.h"

kernel void mult(global const float *a, global const float *b, global float *c, int const M, int const K, int const N) {
    size_t i = get_local_id(0);
    size_t j= get_local_id(1) * WPT;

    size_t global_i = get_global_id(0);
    size_t global_j = get_global_id(1) * WPT;

    local float local_a[TILE_SIZE][TILE_SIZE];
    local float local_b[TILE_SIZE][TILE_SIZE];

    float tmp_sum[WPT];
    for (int k= 0; k < WPT; ++k) tmp_sum[k] = 0.0;

    for (int t = 0; t < K; t += TILE_SIZE) {
        for (int k = 0; k < WPT; ++k) {
            local_a[i][j + k] = a[global_i * K + t + j + k];
            local_b[j + k][i] = b[(t + i) * N + global_j + k];
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        for (int k = 0; k < WPT; ++k) {
            for (int d = 0; d < TILE_SIZE; ++d) {
                tmp_sum[k] += local_a[i][d] * local_b[j + k][d];
            }
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }
    for (int k = 0; k < WPT; ++k) {
        c[global_i * N + global_j + k] = tmp_sum[k];
    }
}