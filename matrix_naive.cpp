//
// Created by imd239 on 22/02/2020.
//

#include <chrono>
#include <iostream>
#include <vector>

using namespace std;
using namespace chrono;

int main() {

    int n;
    cin >> n;

    auto u = vector<vector<int>>(n, vector<int>(n));
    auto v = vector<vector<int>>(n, vector<int>(n));
    auto w = vector<vector<int>>(n, vector<int>(n, 0));

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

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                w[i][j] += u[i][k] * v[k][j];
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

    cout << '\n';


    cout << elapsed << " ms elapsed\n";
}