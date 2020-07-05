kernel void add(global const float8 *a, global const float8 *b, global float8 *c) {
    *c = *a + *b;
}