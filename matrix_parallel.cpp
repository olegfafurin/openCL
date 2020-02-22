//
// Created by imd239 on 22/02/2020.
//

#include <chrono>
#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;
using namespace chrono;

int main() {

    int n;
    cin >> n;

    auto u = vector<vector<int>>(n, vector<int>(n));
    auto v = vector<vector<int>>(n, vector<int>(n));
    auto w = vector<vector<int>>(n, vector<int>(n, 0));
    auto temp = vector<vector<vector<int>>>(8, vector<vector<int>>(n, vector<int>(n, 0)));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> u[i][j];
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> v[i][j];
        }
    }

    high_resolution_clock::time_point t_start = high_resolution_clock::now();

#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < n; ++i)
        {
            int current_thread = omp_get_thread_num();
            for (int j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
//#pragma omp critical
//                    {
                        temp[current_thread][i][j] += u[i][k] * v[k][j];
//                    };
                }
            }
        }
    };

    for (int t = 0; t < 8; ++t) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                w[i][j] += temp[t][i][j];
            }
        }
    }

    high_resolution_clock::time_point t_end = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(t_end - t_start).count();

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cout << w[i][j] << ' ';
        }
        cout << '\n';
    }

    cout << elapsed << " ms elapsed\n";

    return 0;
}