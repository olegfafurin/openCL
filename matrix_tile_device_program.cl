kernel void mult(global const int *u, global const int *v, global int *w, int const a, int const b, int const c) {
    size_t i = get_local_id(0);
    size_t j = get_local_id(1);
    size_t global_i = get_global_id(0);
    size_t global_j = get_global_id(1);

    local int local_a[16][16];
    local int local_b[16][16];

    int tile_size = 16;

    int tmp_sum = 0;

    for (int t = 0; t < b; t += tile_size) {
        local_a[i][j] = u[global_i * b + global_j];
        local_b[j][i] = v[global_j * c + global_i];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (int d = 0; d < tile_size; ++d) {
            tmp_sum += local_a[i][d] * local_b[d][j];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    w[global_i * a + global_j] = tmp_sum;
}