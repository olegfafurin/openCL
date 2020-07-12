//
// Created by imd on 10.07.2020.
//

#include <CL/opencl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "cmath"
#include <algorithm>
#include <random>
#include "util.h"

#define CL_TARGET_OPENCL_VERSION 120

using namespace std;

auto EPS = 0.001;

tuple<bool, int, int, cl_device_type>
get_device(cl_uint &platform_cnt, cl_platform_id *platforms, cl_device_type device_type, bool require_discrete_gpu) {
    for (int i = 0; i < platform_cnt; ++i) {
        cl_uint device_cnt = 0;
        clGetDeviceIDs(platforms[i], device_type, 0, nullptr, &device_cnt);
        auto devices = (cl_device_id *) malloc(device_cnt * sizeof(cl_device_id));
        clGetDeviceIDs(platforms[i], device_type, platform_cnt, devices, &device_cnt);
        if (device_cnt > 0) {
            if (require_discrete_gpu) {
                for (cl_uint j = 0; j < device_cnt; ++j) {
                    size_t l;
                    clGetDeviceInfo(devices[j], CL_DEVICE_HOST_UNIFIED_MEMORY, 0, nullptr, &l);
                    auto is_gpu = (cl_bool *) malloc(l * sizeof(char));
                    clGetDeviceInfo(devices[j], CL_DEVICE_HOST_UNIFIED_MEMORY, l, is_gpu, &l);
                    if (*is_gpu == CL_FALSE) {
                        size_t l1;
                        clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, nullptr, &l1);
                        auto device = (char *) malloc(l1 * sizeof(char));
                        clGetDeviceInfo(devices[j], CL_DEVICE_NAME, l1, device, &l1);
                        cout << device << '\n';
                        free(device);
                        free(is_gpu);
                        free(devices);
                        return make_tuple(true, i, j, device_type);
                    }
                    free(is_gpu);
                }
            } else {
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

    tie(flag, platform_index, device_index, device_type) = get_device(platform_cnt, platforms, CL_DEVICE_TYPE_GPU,
                                                                      true);
    if (!flag)
        tie(flag, platform_index, device_index, device_type) = get_device(platform_cnt, platforms, CL_DEVICE_TYPE_GPU,
                                                                          false);
    if (!flag)
        tie(flag, platform_index, device_index, device_type) = get_device(platform_cnt, platforms, CL_DEVICE_TYPE_CPU,
                                                                          false);

    clGetDeviceIDs(platforms[platform_index], device_type, 0, 0, &device_cnt);
    auto devices = (cl_device_id *) malloc(device_cnt * sizeof(cl_device_id));
    clGetDeviceIDs(platforms[platform_index], device_type, platform_cnt, devices, &device_cnt);
    auto context = clCreateContext(0, 1, devices, nullptr, nullptr, nullptr);

    auto queue = clCreateCommandQueue(context, devices[device_index], CL_QUEUE_PROFILING_ENABLE, nullptr);

    ifstream fin("matrix_opencl_mwpt_device_program.cl");
    vector<char> text(3500, 0);
    fin.read(text.data(), 3500);

    char const *data = text.data();
    const size_t len = strlen(data);

    auto device_prog = clCreateProgramWithSource(context, 1, &data, &len, nullptr);
    cl_int err = clBuildProgram(device_prog, 1, devices, "", nullptr, nullptr);
    if (err != 0) {
        cout << "Device code build error: \n";
        size_t s;
        clGetProgramBuildInfo(device_prog, devices[0], CL_PROGRAM_BUILD_LOG, 0, 0, &s);
        auto ptr4 = (char *) malloc(s * sizeof(char));
        clGetProgramBuildInfo(device_prog, devices[0], CL_PROGRAM_BUILD_LOG, s, ptr4, &s);
        cout << ptr4;
        free(ptr4);
        exit(err);
    } else {
        cout << "Device code has been built successfully\n";
    }

    cl_kernel kernel = clCreateKernel(device_prog, "mult", &err);

    if (err == 0) cout << "Kernel has been created successfully\n";
    else {
        cout << "Kernel creation error: exit code " << err << '\n';
        exit(err);
    }

    size_t a = 2048, b = 512, c = 1024;

    auto *u = new float[a * b];
    auto *v = new float[b * c];
    auto *w = new float[a * c];

    cl_float8 vec{};

    random_device rd;
    uniform_real_distribution<float> ud(0.0, 1.0);
    mt19937 mt(rd());

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < b; ++j) {
            u[i * b + j] = ud(mt);
        }
    }
    for (int i = 0; i < b; ++i) {
        for (int j = 0; j < c; ++j) {
            v[i * c + j] = ud(mt);
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
    if (clEnqueueWriteBuffer(queue, buf2, false, 0, sizeof(float) * b * c, v, 0, 0, 0) != 0) cout << "enq buff error";

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf1);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf2);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf3);
    clSetKernelArg(kernel, 3, sizeof(cl_uint), &a);
    clSetKernelArg(kernel, 4, sizeof(cl_uint), &b);
    clSetKernelArg(kernel, 5, sizeof(cl_uint), &c);
    size_t const dim1 = a / WPT;
    size_t const dim2 = c;
    size_t work_offset[2] = {0, 0};
    size_t work_size[2] = {dim2, dim1};
    size_t const local_dim1 = TILE_SIZE / WPT;
    size_t const local_dim2 = TILE_SIZE;
    size_t local_work_size[2] = {local_dim2, local_dim1};
    cl_event log;
    err = clEnqueueNDRangeKernel(queue, kernel, 2, work_offset, work_size, local_work_size, 0, 0, &log);
    if (err != 0) {
        return err;
    }
    clEnqueueReadBuffer(queue, buf3, true, 0, sizeof(float) * a * c, w, 0, 0, 0);
    cl_ulong t_start, t_end;
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &t_start, nullptr);
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &t_end, nullptr);


    auto *check = new float[a * c];

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < c; ++j) {
            check[i * c + j] = 0;
        }
    }
    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < c; ++j) {
            for (int t = 0; t < b; ++t) {
                check[i * c + j] += u[i * b + t] * v[t * c + j];
            }
        }
    }

    bool correct = true;

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < c; ++j) {
            float diff = check[i * c + j] - w[i * c + j];
            float abs_diff = fabsf(diff);
            if (abs_diff > EPS) {
                correct = false;
            }
        }
    }
    if (!correct) {
        cerr << "Inconsistent results\n";
    }

    cerr << t_end - t_start << " ns elapsed\n";
    double gflops = (double) a * (double) b * (double) c * 2 / (double) (t_end - t_start) * 1'000'000'000;
    cerr << gflops << " FLOPS\n";

    delete [] u;
    delete [] v;
    delete [] w;
    delete [] check;

    free(devices);
    free(platforms);
    return 0;
}
