#pragma region library declararions
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>
#include <Windows.h>
#include "CL\cl.h"
#pragma endregion

#pragma region opencl version defs
#define OPENCL_VERSION_1_2  1.2f
#define OPENCL_VERSION_2_0  2.0f
#define CL_TARGET_OPENCL_VERSION 220
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#pragma endregion

#pragma region opencl object holders
struct ocl_args_d_t {
    ocl_args_d_t();
    ~ocl_args_d_t();
    cl_context       context;
    cl_device_id     device;
    cl_command_queue commandQueue;
    cl_program       program;
    cl_kernel        kernel;
    float            platformVersion;
    float            deviceVersion;
    float            compilerVersion;
    cl_mem           sourceArr;
    cl_mem           averages;
    cl_mem           inputArr;
};
ocl_args_d_t::ocl_args_d_t() :
    context(NULL),
    device(NULL),
    commandQueue(NULL),
    program(NULL),
    kernel(NULL),
    platformVersion(OPENCL_VERSION_1_2),
    deviceVersion(OPENCL_VERSION_1_2),
    compilerVersion(OPENCL_VERSION_1_2),
    sourceArr(NULL),
    averages(NULL),
    inputArr(NULL)
{
}
ocl_args_d_t::~ocl_args_d_t() {
    cl_int err = CL_SUCCESS;
    if (kernel) err = clReleaseKernel(kernel);
    if (program) err = clReleaseProgram(program);
    if (sourceArr) err = clReleaseMemObject(sourceArr);
    if (averages) err = clReleaseMemObject(averages);
    if (inputArr) err = clReleaseMemObject(inputArr);
    if (commandQueue) err = clReleaseCommandQueue(commandQueue);
    if (device) err = clReleaseDevice(device);
}
#pragma endregion

#pragma region device boilderplate checks
void CheckPreferredPlatformMatch(cl_platform_id platform, const char* preferredPlatform) {
    size_t stringLength = 0;
    bool match = false;
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &stringLength);
    std::vector<char> platformName(stringLength);
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, stringLength, &platformName[0], NULL);
    if (strstr(&platformName[0], preferredPlatform) != 0) match = true;
}
cl_platform_id FindOpenCLPlatform(const char* preferredPlatform, cl_device_type deviceType) {
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, NULL, &numPlatforms);
    if (0 == numPlatforms) return NULL;
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
    for (cl_uint i = 0; i < numPlatforms; i++) {
        bool match = true;
        cl_uint numDevices = 0;
        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0)) CheckPreferredPlatformMatch(platforms[i], preferredPlatform);
        if (match) {
            clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);
            if (0 != numDevices) return platforms[i];
        }
    }
    return NULL;
}
void GetPlatformAndDeviceVersion(cl_platform_id platformId, ocl_args_d_t* ocl) {
    size_t stringLength = 0;
    clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, 0, NULL, &stringLength);
    std::vector<char> platformVersion(stringLength);
    clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, stringLength, &platformVersion[0], NULL);
    if (strstr(&platformVersion[0], "OpenCL 2.0") != NULL) ocl->platformVersion = OPENCL_VERSION_2_0;
    clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, 0, NULL, &stringLength);
    std::vector<char> deviceVersion(stringLength);
    clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, stringLength, &deviceVersion[0], NULL);
    if (strstr(&deviceVersion[0], "OpenCL 2.0") != NULL) ocl->deviceVersion = OPENCL_VERSION_2_0;
    clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &stringLength);
    std::vector<char> compilerVersion(stringLength);
    clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, stringLength, &compilerVersion[0], NULL);
    if (strstr(&compilerVersion[0], "OpenCL C 2.0") != NULL) ocl->compilerVersion = OPENCL_VERSION_2_0;
}
#pragma endregion

void generateInput(cl_int* inputArr, cl_uint arrayWidth, cl_uint arrayHeight) {
    srand(67);
    int pm;
    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i) {
        pm = rand() % 3;
        switch (pm) {
        case 0:
            inputArr[i] = -1;
            break;
        case 1:
            inputArr[i] = 1;
            break;
        case 2:
            inputArr[i] = 0;
            break;
        }
        
    }
}

#pragma region opencl program creation
void SetupOpenCL(ocl_args_d_t* ocl, cl_device_type deviceType) {
    cl_int err = CL_SUCCESS;
    cl_platform_id platformId = FindOpenCLPlatform("Nvidia", deviceType);
    cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0 };
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &err);
    err = clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    GetPlatformAndDeviceVersion(platformId, ocl);
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion) {
        const cl_command_queue_properties properties[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
        ocl->commandQueue = clCreateCommandQueueWithProperties(ocl->context, ocl->device, properties, &err);
    }
    else {
        cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
        ocl->commandQueue = clCreateCommandQueue(ocl->context, ocl->device, properties, &err);
    }
}
int ReadSourceFromFile(const char* fileName, char** source, size_t* sourceSize) {
    int errorCode = CL_SUCCESS;
    FILE* fp = NULL;
    fopen_s(&fp, fileName, "rb");
    if (fp == NULL) errorCode = CL_INVALID_VALUE;
    else {
        fseek(fp, 0, SEEK_END);
        *sourceSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        *source = new char[*sourceSize];
        if (*source == NULL) errorCode = CL_OUT_OF_HOST_MEMORY;
        else fread(*source, 1, *sourceSize, fp);
    }
    return errorCode;
}
void CreateAndBuildProgram(ocl_args_d_t* ocl) {
    char* source = NULL;
    size_t src_size = 0;
    ReadSourceFromFile("simple.cl", &source, &src_size);
    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, NULL);
    clBuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL);
}
#pragma endregion

#pragma region kernel handling
void CreateBufferArguments(ocl_args_d_t* ocl, cl_int* inputArr, cl_float* averagesInput, cl_int* outArr, cl_uint arrayWidth, cl_uint arrayHeight) {
    unsigned int size = arrayHeight * arrayWidth;
    ocl->sourceArr = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size * sizeof(int), inputArr, NULL);
    ocl->averages = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 4 * sizeof(float), averagesInput, NULL);
    clEnqueueWriteBuffer(ocl->commandQueue, ocl->sourceArr, CL_TRUE, 0, size * sizeof(int), inputArr, 0, NULL, NULL);
    clEnqueueWriteBuffer(ocl->commandQueue, ocl->averages, CL_TRUE, 0, 4 * sizeof(float), averagesInput, 0, NULL, NULL);
    ocl->inputArr = clCreateBuffer(ocl->context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, size * sizeof(int), outArr, NULL);
}
void SetKernelArguments(ocl_args_d_t* ocl) {
    clSetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void*)&ocl->sourceArr);
    clSetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void*)&ocl->averages);
    clSetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void*)&ocl->inputArr);
}
void ExecuteAddKernel(ocl_args_d_t* ocl, cl_uint width, cl_uint height) {
    size_t global_item_size = width * height;
    size_t local_item_size = 64; // Divide work items into groups of 64
    clEnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    clFinish(ocl->commandQueue);
}
#pragma endregion

void calculateGlobalAverages(float* averagesArray, cl_int* inputArr, int arrayHeight, int arrayWidth) {
    int i;
    for (i = 0; i < 4; i++) averagesArray[i] = (float)0.0f;
    for (i = 0; i < arrayHeight * arrayWidth; i++) {
        for (int j = 2; j < 4; j++) if ((i + j) % 4) averagesArray[j] += (float)inputArr[i];
    }
    for (i = 2; i < 4; i++) averagesArray[i] = averagesArray[i] / (arrayHeight * arrayWidth);
}

int _tmain(int argc, TCHAR* argv[]) {
    
    ///
    /// performance analysis
    ///

    LARGE_INTEGER perfFrequency;
    LARGE_INTEGER performanceCountNDRangeStart;
    LARGE_INTEGER performanceCountNDRangeStop;
    bool queueProfilingEnable = true;
    if (queueProfilingEnable) QueryPerformanceCounter(&performanceCountNDRangeStart);

    ///
    /// main program start
    ///

    ocl_args_d_t ocl;
    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;
    // arrayWidth and arrayHeight must be powers of two
    cl_uint arrayWidth = 16;
    // each group of 4 pixels wide represents: isAlive, willRebound, velocity x, velocity y: in this order
    cl_uint arrayHeight = arrayWidth / 4;
    cl_int size = arrayHeight * arrayWidth;
    // opencl setup
    SetupOpenCL(&ocl, deviceType);
    CreateAndBuildProgram(&ocl);
    ocl.kernel = clCreateKernel(ocl.program, "Add", NULL);
    // problem variables
    cl_int* inputArr = (cl_int*)malloc(sizeof(int) * size);
    cl_float* averagesArray = (cl_float*)malloc(sizeof(float) * 4);
    cl_int* outArr = (cl_int*)malloc(sizeof(int) * size);
    generateInput(inputArr, arrayWidth, arrayHeight);
    calculateGlobalAverages(averagesArray, inputArr, arrayWidth, arrayHeight);

    ///
    /// solution
    ///

    printf("in:\n\n");
    for (int k = 0; k < size; k++) printf("%d ", inputArr[k]);
    printf("\n\n");
    printf("changes:\n\n");
    int iteration_count = 3;
    for (int i = 0; i < iteration_count; i++) {
        // take inputs from previous buffer, set them as new buffer
        CreateBufferArguments(&ocl, inputArr, averagesArray, outArr, arrayWidth, arrayHeight);
        // execute kernel
        SetKernelArguments(&ocl);
        ExecuteAddKernel(&ocl, arrayWidth, arrayHeight);
        // read kernel outputs back into host buffer
        clEnqueueReadBuffer(ocl.commandQueue, ocl.inputArr, CL_TRUE, 0, size * sizeof(int), inputArr, 0, NULL, NULL);
        // adjust averages from previous buffer
        calculateGlobalAverages(averagesArray, inputArr, arrayHeight, arrayWidth);
        // debug
        for (int k = 0; k < size; k++) printf("%d ", inputArr[k]);
        printf("\n\n");
    }

    ///
    /// finish main program, print benchmarking
    ///

    printf("out:\n\n");
    for (int k = 0; k < size; k++) printf("%d ", inputArr[k]);
    printf("\n\n");
    // finish program
    clFinish(ocl.commandQueue);
    free(inputArr);
    free(outArr);

    ///
    /// print benchmarking results
    ///

    if (queueProfilingEnable) QueryPerformanceCounter(&performanceCountNDRangeStop);
    if (queueProfilingEnable) QueryPerformanceFrequency(&perfFrequency);
    printf("success: execution time %f ms.\n", 1000.0f * (float)(performanceCountNDRangeStop.QuadPart - performanceCountNDRangeStart.QuadPart) / (float)perfFrequency.QuadPart);
    return 0;
}
