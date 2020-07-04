//
// Created by imd239 on 22/02/2020.
//

#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <omp.h>

using namespace std;
using namespace chrono;

int main() {

    int n, m, k;
    cin >> n >> k >> m;

    float u[n][k];
    float v[k][m];
    float w[n][m];
    float temp[8][n][m];

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int l = 0; l < m; ++l) {
                temp[i][j][l] = 0.0;
            }
        }
    }
    for (int j = 0; j < n; ++j) {
        for (int l = 0; l < m; ++l) {
            w[j][l] = 0.0;
        }
    }

    random_device rd;
    uniform_real_distribution<float> ud(0.0, 1.0);
    mt19937 mt(rd());

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            u[i][j] = ud(mt);
        }
    }

    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < m; ++j) {
            v[i][j] = ud(mt);
        }
    }

    high_resolution_clock::time_point t_start = high_resolution_clock::now();

#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < n; ++i)
        {
            int current_thread = omp_get_thread_num();
            for (int j = 0; j < m; ++j) {
                for (int t = 0; t < k; ++t) {
//#pragma omp critical
//                    {
                        temp[current_thread][i][j] += u[i][t] * v[t][j];
//                    };
                }
            }
        }
    };

    for (int t = 0; t < 8; ++t) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                w[i][j] += temp[t][i][j];
            }
        }
    }

    high_resolution_clock::time_point t_end = high_resolution_clock::now();
    auto elapsed = duration_cast<nanoseconds>(t_end - t_start).count();

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cout << w[i][j] << ' ';
        }
        cout << '\n';
    }

    cerr << elapsed << " ns elapsed\n";

    return 0;
}