#include "util.h"

kernel void prefix_sum(global const float *arr, global float *res) {
    int i = get_global_id(0);
    local float a[PREFIX_SUM_WORK_SIZE];

    a[i] = arr[i];
    float mem;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (int t = 1; i >= t; t <<= 1) {
        mem = a[i - t];
        barrier(CLK_LOCAL_MEM_FENCE);
        a[i] +=  mem;
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    res[i] = a[i];
}