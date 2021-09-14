#pragma region library declararions
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>
#include <Windows.h>
#include <math.h>
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

    cl_context       context;           // hold the context handler
    cl_device_id     device;            // hold the selected device handler
    cl_command_queue commandQueue;      // hold the commands-queue handler
    cl_program       program;           // hold the program handler
    cl_kernel        kernel;            // hold the kernel handler
    float            platformVersion;   // hold the OpenCL platform version (default 1.2)
    float            deviceVersion;     // hold the OpenCL device version (default. 1.2)
    float            compilerVersion;   // hold the device OpenCL C version (default. 1.2)

    cl_mem           srcA;              // hold first source buffer
    cl_mem           srcB;              // hold second source buffer
    cl_mem           averages;           // hold previous averages
    cl_mem           dstMem;            // hold destination buffer
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
    srcA(NULL),
    srcB(NULL),
    averages(NULL),
    dstMem(NULL)
{
}
ocl_args_d_t::~ocl_args_d_t() {
    cl_int err = CL_SUCCESS;
    if (kernel) err = clReleaseKernel(kernel);
    if (program) err = clReleaseProgram(program);
    if (srcA) err = clReleaseMemObject(srcA);
    if (srcB) err = clReleaseMemObject(srcB);
    if (averages) err = clReleaseMemObject(averages);
    if (dstMem) err = clReleaseMemObject(dstMem);
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
    for (cl_uint i = 0; i < numPlatforms; i++){
        bool match = true;
        cl_uint numDevices = 0;
        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0)) CheckPreferredPlatformMatch(platforms[i], preferredPlatform);
        if (match){
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

#pragma region initialise inputs
void generateInput(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight) {
    srand(34);
    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i) inputArray[i] = rand() % 2;
}
void generateInput_(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight) {
    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i) inputArray[i] = 1;
}
void generateInput__(cl_int* inputArray) {
    for (int i = 0; i < 4; i++) inputArray[i] = 0;
}
#pragma endregion

#pragma region opencl program creation
void SetupOpenCL(ocl_args_d_t* ocl, cl_device_type deviceType) {
    cl_int err = CL_SUCCESS;
    cl_platform_id platformId = FindOpenCLPlatform("Nvidia", deviceType);
    cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0 };
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &err);
    err = clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    GetPlatformAndDeviceVersion(platformId, ocl);
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion){
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
void CreateBufferArguments(ocl_args_d_t* ocl, cl_int* inputA, cl_int* inputB, cl_int* outputC, cl_int* averagesInput, cl_uint arrayWidth, cl_uint arrayHeight) {
    unsigned int size = arrayHeight * arrayWidth;
    ocl->srcA = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size * sizeof(int), inputA, NULL);
    ocl->srcB = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size * sizeof(int), inputB, NULL);
    ocl->averages = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 4 * sizeof(int), averagesInput, NULL);
    clEnqueueWriteBuffer(ocl->commandQueue, ocl->srcA, CL_TRUE, 0, size * sizeof(int), inputA, 0, NULL, NULL);
    clEnqueueWriteBuffer(ocl->commandQueue, ocl->srcB, CL_TRUE, 0, size * sizeof(int), inputB, 0, NULL, NULL);
    clEnqueueWriteBuffer(ocl->commandQueue, ocl->averages, CL_TRUE, 0, 4 * sizeof(int), averagesInput, 0, NULL, NULL);
    ocl->dstMem = clCreateBuffer(ocl->context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, size * sizeof(int), outputC, NULL);
}
void SetKernelArguments(ocl_args_d_t* ocl) {
    clSetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void*)&ocl->srcA);
    clSetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void*)&ocl->srcB);
    clSetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void*)&ocl->averages);
    clSetKernelArg(ocl->kernel, 3, sizeof(cl_mem), (void*)&ocl->dstMem);
}
void ExecuteAddKernel(ocl_args_d_t* ocl, cl_uint width, cl_uint height) {
    size_t global_item_size = width * height;
    size_t local_item_size = 64; // Divide work items into groups of 64
    clEnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    clFinish(ocl->commandQueue);
}
#pragma endregion

void calculateAverages(cl_int* averagesArray, cl_int* inputA, int arrayHeight, int arrayWidth) {
    for (int i = 0; i < 4; i++) averagesArray[i] = 0;
    for (int i = 0; i < arrayHeight * arrayWidth; i++) {
        if (i % 4 == 0) averagesArray[0] += inputA[i];
        else if ((i + 1) % 4) averagesArray[1] += inputA[i];
        else if ((i + 2) % 4) averagesArray[2] += inputA[i];
        else if ((i + 3) % 4) averagesArray[3] += inputA[i];
    }
    averagesArray[0] = (int)((float)averagesArray[0] / (float)(arrayHeight * arrayWidth));
    averagesArray[1] = (int)((float)averagesArray[1] / (float)(arrayHeight * arrayWidth));
    averagesArray[2] = (int)((float)averagesArray[2] / (float)(arrayHeight * arrayWidth));
    averagesArray[3] = (int)((float)averagesArray[3] / (float)(arrayHeight * arrayWidth));
}

int _tmain(int argc, TCHAR* argv[]) {
    // performance analysis
    LARGE_INTEGER perfFrequency;
    LARGE_INTEGER performanceCountNDRangeStart;
    LARGE_INTEGER performanceCountNDRangeStop;
    bool queueProfilingEnable = true;
    if (queueProfilingEnable) QueryPerformanceCounter(&performanceCountNDRangeStart);

    // setup
    ocl_args_d_t ocl;
    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

    cl_uint arrayWidth = 16;
    cl_uint arrayHeight = arrayWidth / 4;
    cl_int size = arrayHeight * arrayWidth;

    SetupOpenCL(&ocl, deviceType);
    CreateAndBuildProgram(&ocl);
    ocl.kernel = clCreateKernel(ocl.program, "Add", NULL);

    cl_int* inputA = (cl_int*)malloc(sizeof(int) * size);
    cl_int* inputB = (cl_int*)malloc(sizeof(int) * size);
    cl_int* averagesArray = (cl_int*)malloc(sizeof(int) * 4);
    cl_int* outputC = (cl_int*)malloc(sizeof(int) * size);
    generateInput(inputA, arrayWidth, arrayHeight);
    generateInput_(inputB, arrayWidth, arrayHeight);
    generateInput__(averagesArray);

    ///
    /// main program: multiple data instantiations sent to GPU, braught back to cpu, sent back to gpu
    ///

    printf("in:\n\n");
    for (int k = 0; k < size; k++) printf("A[%d]: %d\n", k, inputA[k]);
    printf("\n");

    int iteration_count = 10;
    for (int i = 0; i < iteration_count; i++) {
        // take inputs from previous buffer, set them as new buffer
        CreateBufferArguments(&ocl, inputA, inputB, outputC, averagesArray, arrayWidth, arrayHeight);

        // execute kernel
        SetKernelArguments(&ocl);
        ExecuteAddKernel(&ocl, arrayWidth, arrayHeight);
        
        // read kernel outputs back into host buffer
        clEnqueueReadBuffer(ocl.commandQueue, ocl.dstMem, CL_TRUE, 0, size * sizeof(int), inputA, 0, NULL, NULL);
        
        // averages
        calculateAverages(averagesArray, inputA, arrayHeight, arrayWidth);
        printf("%d %d %d %d\n", averagesArray[0], averagesArray[1], averagesArray[2], averagesArray[3]);
    }

    ///
    /// finish main program, print benchmarking
    ///

    printf("out:\n\n");
    for (int k = 0; k < size; k++) printf("A[%d]: %d\n", k, inputA[k]);

    printf("\n\nread success\n");

    if (queueProfilingEnable) QueryPerformanceCounter(&performanceCountNDRangeStop);
    if (queueProfilingEnable) QueryPerformanceFrequency(&perfFrequency);
    printf("\nperformance counter time %f ms.\n\n", 1000.0f * (float)(performanceCountNDRangeStop.QuadPart - performanceCountNDRangeStart.QuadPart) / (float)perfFrequency.QuadPart);

    clFinish(ocl.commandQueue);
    free(inputA);
    free(inputB);
    free(outputC);

    return 0;
}
