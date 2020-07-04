//
// Created by imd239 on 07/03/2020.
//

#include <CL/opencl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <random>
#include "util.h"

using namespace std;

auto GPU_NVIDIA = "NVIDIA";
auto GPU_AMD = "AMD";

tuple<bool, int, int, cl_device_type> get_device(cl_uint &platform_cnt, cl_platform_id * platforms, cl_device_type device_type, bool require_discrete_gpu) {
    for (int i = 0; i < platform_cnt; ++i) {
        cl_uint device_cnt = 0;
        clGetDeviceIDs(platforms[i], device_type, 0, nullptr, &device_cnt);
        auto devices = (cl_device_id *) malloc(device_cnt * sizeof(cl_device_id));
        clGetDeviceIDs(platforms[i], device_type, platform_cnt, devices, &device_cnt);
        if (device_cnt > 0) {
            if (require_discrete_gpu) {
                for (cl_uint j = 0; j < device_cnt; ++j) {
                    size_t l;
                    clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, nullptr, &l);
                    auto device = (char *) malloc(l * sizeof(char));
                    clGetDeviceInfo(devices[j], CL_DEVICE_NAME, l, device, &l);
                    if (strstr(device, GPU_AMD) != nullptr || strstr(device, GPU_NVIDIA) != nullptr) {
                        cout << device << '\n';
                        free(device);
                        free(devices);
                        return make_tuple(true, i, j, device_type);
                    }
                    free(device);
                }
            }
            else {
                size_t l;
                clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, nullptr, &l);
                auto device = (char *) malloc(l * sizeof(char));
                clGetDeviceInfo(devices[0], CL_DEVICE_NAME, l, device, &l);
                cout << device << '\n';
                free(device);
                free(devices);
                return make_tuple(true, i, 0, device_type);
            }
        }
        free(devices);
    }
    return make_tuple(false, 0, 0, device_type);
}

int main() {
    cl_uint platform_cnt, device_cnt;
    clGetPlatformIDs(0, nullptr, &platform_cnt);
    auto platforms = (cl_platform_id *) malloc(platform_cnt * sizeof(cl_platform_id));
    clGetPlatformIDs(platform_cnt, platforms, &platform_cnt);

    bool flag;
    cl_platform_id platform;
    cl_device_id device;
    int platform_index, device_index;
    cl_device_type device_type;

    tie(flag, platform_index, device_index, device_type) = get_device(platform_cnt, platforms, CL_DEVICE_TYPE_GPU, true);
    if (!flag) tie(flag, platform_index, device_index, device_type) = get_device(platform_cnt, platforms, CL_DEVICE_TYPE_GPU, false);
    if (!flag) tie(flag, platform_index, device_index, device_type) = get_device(platform_cnt, platforms, CL_DEVICE_TYPE_CPU, false);

    clGetDeviceIDs(platforms[platform_index], device_type, 0, 0, &device_cnt);
    auto devices = (cl_device_id *) malloc(device_cnt * sizeof(cl_device_id));
    clGetDeviceIDs(platforms[platform_index], device_type, platform_cnt, devices, &device_cnt);
    auto context = clCreateContext(0, 1, devices, nullptr, nullptr, nullptr);

    auto queue = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE, nullptr);

    ifstream fin("matrix_tile_device_program.cl");
    vector<char> text(1024, 0);
    fin.read(text.data(), 1024);

    char const *data = text.data();
    const size_t len = strlen(data);

    auto device_prog = clCreateProgramWithSource(context, 1, &data, &len, nullptr);
    cl_int err = clBuildProgram(device_prog, 1, devices, "", nullptr, nullptr);
    if (err != 0) {
        cout << "fucked up\n";
        size_t s;
        clGetProgramBuildInfo(device_prog, devices[0], CL_PROGRAM_BUILD_LOG, 0, 0, &s);
        auto ptr4 = (char *) malloc(s * sizeof(char));
        clGetProgramBuildInfo(device_prog, devices[0], CL_PROGRAM_BUILD_LOG, s, ptr4, &s);
        cout << ptr4;
        free(ptr4);
        exit(err);
    } else {
        cout << "success\n";
    }

    cl_kernel kernel = clCreateKernel(device_prog, "mult", &err);

    cout << err << '\n';

    cl_int a, b, c;
    cin >> a >> b >> c;
    float u[a][b];
    float v[b][c];
    float w[a][c];

    random_device rd;
    uniform_real_distribution<float> ud(0.0, 1.0);
    mt19937 mt(rd());

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < b; ++j) {
            u[i][j] = ud(mt);
        }
    }
    for (int i = 0; i < b; ++i) {
        for (int j = 0; j < c; ++j) {
            v[i][j] = ud(mt);
        }
    }

    auto buf1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * a * b, 0, &err);
    if (err != 0) return err;
    auto buf2 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * b * c, 0, &err);
    if (err != 0) return err;
    auto buf3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * a * c, 0, &err);
    if (err != 0) return err;

    if (!buf1 || !buf2 || !buf3) cout << "buffer allocation error";

    if (clEnqueueWriteBuffer(queue, buf1, false, 0, sizeof(float) * a * b, u, 0, 0, 0) != 0) cout << "enq buff error";
    if (clEnqueueWriteBuffer(queue, buf2, false, 0, sizeof(float) * b * c, v, 0, 0, 0) != 0)  cout << "enq buff error";

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf1);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf2);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf3);
    clSetKernelArg(kernel, 3, sizeof(cl_uint), &a);
    clSetKernelArg(kernel, 4, sizeof(cl_uint), &b);
    clSetKernelArg(kernel, 5, sizeof(cl_uint), &c);


    size_t const dim1 = a;
    size_t const dim2 = c;
    size_t work_offset[2] = {0, 0};
    size_t work_size[2] = {dim1, dim2};
    cl_event log;
    err = clEnqueueNDRangeKernel(queue, kernel, 2, work_offset, work_size, nullptr, 0, 0, &log);
    if (err != 0){
        return err;
    }
    clEnqueueReadBuffer(queue, buf3, true, 0, sizeof(float) * a * c, &w, 0, 0, 0);
    cl_ulong t_start, t_end;
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &t_start, nullptr);
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &t_end, nullptr);

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < c; ++j) {
            cout << w[i][j] << ' ';
        }
        cout << '\n';
    }

    cerr << t_end - t_start << " ns elapsed\n";

    free(devices);
    free(platforms);
}