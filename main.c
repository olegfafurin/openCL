//
// Created by imd239 on 2/15/20.
//
#include "stdio.h"
#include <omp.h>
#include <math.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

bool isPrime(int k) {
    for (int j = 2; j * j <= k; j++) {
        if (k % j == 0) return false;
    }
    return true;
}

int main() {

    int n = 10000000;
    int counter[8];
    for (int i = 0; i < 8; ++i) {
        counter[i] = 0;
    }
//    scanf("%d", &n);
#pragma omp parallel
    {
        int k = omp_get_thread_num();
        printf("%d\n", k);
#pragma omp for
        for (int i = 3; i < n; i += 2) {
            if (isPrime(i)) counter[k]++;
        }
    }
    int cnt = 1;
    for (int i = 0; i < 8; ++i) {
        cnt += counter[i];
    }
    printf("%d\n", cnt);
    for (int i = 0; i < 8; ++i) {
        printf("%d\n", counter[i]);
    }
}