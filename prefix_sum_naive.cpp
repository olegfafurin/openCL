//
// Created by imd on 7/5/20.
//

#include <random>
#include <iostream>
#include <chrono>
#include "util.h"

using namespace std;
using namespace chrono;

int main() {

    random_device rd;
    uniform_real_distribution<float> ud(0.0, 1.0);
    mt19937 mt(rd());

    int n = PREFIX_SUM_WORK_SIZE;

    float arr[PREFIX_SUM_WORK_SIZE];
    for (int i = 0; i < PREFIX_SUM_WORK_SIZE; ++i)  arr[i] = ud(mt);

    high_resolution_clock::time_point t_start = high_resolution_clock::now();

    float check[n];
    check[0] = arr[0];
    for (int i = 1; i < n; ++i) {
        check[i] = check[i - 1] + arr[i];
    }

    high_resolution_clock::time_point t_end = high_resolution_clock::now();
    auto elapsed = duration_cast<nanoseconds>(t_end - t_start).count();

    cerr << elapsed << " ns elapsed\n";
    double gflops = (double) n * log2((double) n) / (double)(elapsed) * 1'000'000'000;
    cerr << gflops << " FLOPS\n";

}