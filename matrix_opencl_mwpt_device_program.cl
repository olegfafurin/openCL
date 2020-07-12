#include "util.h"

__kernel void mult(__global const float *a, __global const float *b, __global float *c, uint const M, uint const K, uint const N) {

    size_t const j = get_local_id(0);
    size_t const i = get_local_id(1) * WPT;

    size_t const global_j = get_global_id(0);
    size_t const global_i = get_global_id(1) * WPT;

    local float local_a[TILE_SIZE][TILE_SIZE];
    local float local_b[TILE_SIZE][TILE_SIZE];

    float tmp_sum[WPT];
    for (int k = 0; k < WPT; ++k) tmp_sum[k] = 0.0;

    for (int t = 0; t < K; t += TILE_SIZE) {
        for (int k = 0; k < WPT; ++k) {
            local_a[i + k][j] = a[(global_i + k) * K + t + j];
            local_b[i + k][j] = b[(t + i + k) * N + global_j];
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        for (int k = 0; k < WPT; ++k) {
            for (int d = 0; d < TILE_SIZE; ++d) {
                tmp_sum[k] += local_a[i + k][d] * local_b[d][j];
            }
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }
    for (int k = 0; k < WPT; ++k) {
        c[(global_i + k) * N + global_j] = tmp_sum[k];
    }
}