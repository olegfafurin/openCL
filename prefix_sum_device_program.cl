#include "util.h"

kernel void prefix_sum(global const float *arr, global float *res) {
    uint i = get_global_id(0);
    local float a[2 * PREFIX_SUM_WORK_SIZE];

    a[i] = arr[i];

    local int h_in;
    local int h_out;

    if (i == 0) {
        h_in = 0;
        h_out = 1;
    }

    for (uint t = 1; PREFIX_SUM_WORK_SIZE > t; t <<= 1) {
        barrier(CLK_LOCAL_MEM_FENCE);
        if (i >= t) {
            a[h_out * PREFIX_SUM_WORK_SIZE + i] = a[h_in * PREFIX_SUM_WORK_SIZE + i] + a[h_in  * PREFIX_SUM_WORK_SIZE + i - t];
        }
        else {
            a[h_out * PREFIX_SUM_WORK_SIZE + i] = a[h_in * PREFIX_SUM_WORK_SIZE + i];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        if (i == 0) {
            h_in = 1 - h_in;
            h_out = 1 - h_out;
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    res[i] = a[i + h_in * PREFIX_SUM_WORK_SIZE];
}