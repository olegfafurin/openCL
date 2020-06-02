kernel void mult(global const int *u, global const int *v, global int *w, int const a, int const b, int const c) {
    size_t i = get_local_id(0);
    size_t j = get_local_id(1);
    size_t global_i = get_global_id(0);
    size_t global_j = get_global_id(1);

    if ((global_i >= a) || (global_j >= b)) return;
    int res = 0;
    for (uint t = 0; t < b; t++) {
        res += u[global_i * b + t] * v[t * b + global_j];
    }
    w[global_i * c + global_j] = res;
}

//local int chunk[16][16];