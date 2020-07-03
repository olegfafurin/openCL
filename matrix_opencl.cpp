//
// Created by imd239 on 07/03/2020.
//

#include <CL/opencl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <chrono>
#include <tuple>

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

    auto queue = clCreateCommandQueue(context, devices[device_index], CL_QUEUE_PROFILING_ENABLE, nullptr);

    ifstream fin("matrix_device_program.cl");
    vector<char> text(1024, 0);
    fin.read(text.data(), 1024);

    char const *data = text.data();
    const size_t len = strlen(data);

    auto device_prog = clCreateProgramWithSource(context, 1, &data, &len, nullptr);
    cl_int err = clBuildProgram(device_prog, 1, devices, "", nullptr, nullptr);
    if (err != 0) {
        cout << "Device code build error: \n";
        size_t s;
        clGetProgramBuildInfo(device_prog, devices[device_index], CL_PROGRAM_BUILD_LOG, 0, 0, &s);
        auto err_trace = (char *) malloc(s * sizeof(char));
        clGetProgramBuildInfo(device_prog, devices[device_index], CL_PROGRAM_BUILD_LOG, s, err_trace, &s);
        cout << err_trace;
        free(err_trace);
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

    cl_int a, b, c;
    cin >> a >> b >> c;
    int u[a][b];
    int v[b][c];
    int w[a][c];

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < b; ++j) {
            cin >> u[i][j];
        }
    }
    for (int i = 0; i < b; ++i) {
        for (int j = 0; j < c; ++j) {
            cin >> v[i][j];
        }
    }

    auto buf1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int) * a * b, 0, &err);
    if (err != 0) return err;
    auto buf2 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int) * b * c, 0, &err);
    if (err != 0) return err;
    auto buf3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * a * c, 0, &err);
    if (err != 0) return err;
//    auto buf1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int), 0, 0);
//    auto buf2 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int), 0, 0);
//    auto buf3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int), 0, 0);
//    auto buf4 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int), 0, 0);
//    auto buf5 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int), 0, 0);
//    auto buf6 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int), 0, 0);

    if (!buf1 || !buf2 || !buf3) cout << "buffer allocation error";

//    cl_int result;
    cl_int tile_size = 16;

    if (clEnqueueWriteBuffer(queue, buf1, false, 0, sizeof(cl_int) * a * b, u, 0, 0, 0) != 0) cout << "enq buff error";
    if (clEnqueueWriteBuffer(queue, buf2, false, 0, sizeof(cl_int) * b * c, v, 0, 0, 0) != 0)  cout << "enq buff error";
//    clEnqueueWriteBuffer(queue, buf3, false, 0, sizeof(cl_int), w, 0, 0, 0);
//    clEnqueueWriteBuffer(queue, buf4, false, 0, sizeof(cl_int), &a, 0, 0, 0);
//    clEnqueueWriteBuffer(queue, buf5, false, 0, sizeof(cl_int), &b, 0, 0, 0);
//    clEnqueueWriteBuffer(queue, buf6, false, 0, sizeof(cl_int), &c, 0, 0, 0);

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
//        clGetEventProfilingInfo(log, )
        return err;
    }
    clEnqueueReadBuffer(queue, buf3, true, 0, sizeof(cl_int) * a * c, &w, 0, 0, 0);
    cl_ulong t_start, t_end;
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &t_start, nullptr);
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &t_end, nullptr);


    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < c; ++j) {
            cout << w[i][j] << ' ';
        }
        cout << '\n';
    }

//    cout << result << '\n';
    cout << t_end - t_start << " ns elapsed";

    free(devices);
    free(platforms);
}