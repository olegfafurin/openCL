//
// Created by imd239 on 22/02/2020.
//

#include <CL/opencl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <chrono>

using namespace std;

int main() {
    cl_uint n;
    clGetPlatformIDs(0, nullptr, &n);
    auto ptr = (cl_platform_id *) malloc(n * sizeof(cl_platform_id));
    clGetPlatformIDs(n, ptr, &n);

    clGetDeviceIDs(ptr[2], CL_DEVICE_TYPE_GPU, 0, 0, &n);
    auto ptr2 = (cl_device_id *) malloc(n * sizeof(cl_device_id));
    clGetDeviceIDs(ptr[2], CL_DEVICE_TYPE_GPU, n, ptr2, &n);

    for (int i = 0; i < n; ++i) {
        size_t m;
        clGetDeviceInfo(ptr2[i], CL_DEVICE_NAME, 0, 0, &m);
        auto ptr3 = (char *) malloc(m * sizeof(char));
        clGetDeviceInfo(ptr2[i], CL_DEVICE_NAME, m, ptr3, &m);
        cout << ptr3 << '\n';
        free(ptr3);
    }

    auto context = clCreateContext(0, 1, ptr2, nullptr, nullptr, nullptr);

    auto queue = clCreateCommandQueue(context, ptr2[0], CL_QUEUE_PROFILING_ENABLE, nullptr);

    ifstream fin("a+b_device_program.cl");
    vector<char> text(1024, 0);
    fin.read(text.data(), 1024);


//    FILE *f = fopen("device_program.cl", "r");
//    char data[1024];
//    fread(data, sizeof(char), 1024, f);


    char const *data = text.data();
    const size_t len = strlen(data);

    auto device_prog = clCreateProgramWithSource(context, 1, &data, &len, nullptr);
    cl_int err = clBuildProgram(device_prog, 1, ptr2, "", nullptr, nullptr);
    if (err != 0) {
        cout << "fucked up\n";
        size_t s;
        clGetProgramBuildInfo(device_prog, ptr2[0], CL_PROGRAM_BUILD_LOG, 0, 0, &s);
        auto ptr4 = (char *) malloc(s * sizeof(char));
        clGetProgramBuildInfo(device_prog, ptr2[0], CL_PROGRAM_BUILD_LOG, s, ptr4, &s);
        cout << ptr4;
        free(ptr4);
        exit(err);
    } else {
        cout << "success\n";
    }


    cl_kernel kernel = clCreateKernel(device_prog, "add", &err);
    auto buf1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float8), 0, 0);
    auto buf2 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float8), 0, 0);
    auto buf3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float8), 0, 0);
    if (!buf1 || !buf2 || !buf3) cout << "buffer allocation error";

    cl_float a, b;
    cin >> a;
    cin >> b;
    cl_float8 result;
    cl_float8 in1, in2;
    for (int i = 0; i < 8; ++i) in1.s[i] = a - static_cast<float>(i);
    for (int i = 0; i < 8; ++i) in2.s[i] = b + static_cast<float>(i);

    clEnqueueWriteBuffer(queue, buf1, false, 0, sizeof(cl_float8), &in1, 0, 0, 0);
    clEnqueueWriteBuffer(queue, buf2, false, 0, sizeof(cl_float8), &in2, 0, 0, 0);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf1);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf2);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf3);
    size_t work_offset = 0;
    size_t work_size = 1;
    cl_event log;
    clEnqueueNDRangeKernel(queue, kernel, 1, &work_offset, &work_size, nullptr, 0, 0, &log);
    clEnqueueReadBuffer(queue, buf3, true, 0, sizeof(cl_float8), &result, 0, 0, 0);
    cl_ulong t_start, t_end;
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &t_start, nullptr);
    clGetEventProfilingInfo(log, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &t_end, nullptr);

//    cout << result << '\n';
    cout << result.s0 << ' ' << result.s1 << ' ' << result.s2 << ' ' << result.s3 << ' ' << result.s4 << ' ' << result.s5 << ' ' << result.s6 << ' ' << result.s7 << '\n';
    cout << t_end - t_start << " ns elapsed";



    free(ptr2);
    free(ptr);
}