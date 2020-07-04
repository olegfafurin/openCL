//
// Created by imd239 on 22/02/2020.
//

#include <chrono>
#include <iostream>
#include <vector>

using namespace std;
using namespace chrono;

int main() {

    int n, m, k;
    cin >> n >> k >> m;

    auto u = vector<vector<int>>(n, vector<int>(k));
    auto v = vector<vector<int>>(k, vector<int>(m));
    auto w = vector<vector<int>>(n, vector<int>(m, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            cin >> u[i][j];
        }
    }

    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> v[i][j];
        }
    }

    high_resolution_clock::time_point t_start = high_resolution_clock::now();

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            for (int t = 0; t < k; ++t) {
                w[i][j] += u[i][t] * v[t][j];
            }
        }
    }
    high_resolution_clock::time_point t_end = high_resolution_clock::now();
    auto elapsed = duration_cast<nanoseconds>(t_end - t_start).count();

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cout << w[i][j] << ' ';
        }
        cout << '\n';
    }

    cout << '\n';
    cerr << elapsed << " ns elapsed\n";
}