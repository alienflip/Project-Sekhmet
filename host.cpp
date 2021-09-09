#pragma region c++ libs/includes, opencl version defs

#define OPENCL_VERSION_1_2  1.2f
#define OPENCL_VERSION_2_0  2.0f
#define CL_TARGET_OPENCL_VERSION 220
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>
#include <Windows.h>

#include "CL\cl.h"

#pragma endregion

#pragma region create / destroy boilerplate
struct ocl_args_d_t
{
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

    cl_mem           srcA;
    cl_mem           srcB;
    cl_mem           dstMem;
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
    dstMem(NULL)
{
}

ocl_args_d_t::~ocl_args_d_t()
{
    cl_int err = CL_SUCCESS;

    if (kernel) err = clReleaseKernel(kernel);
    if (program) err = clReleaseProgram(program);
    if (srcA) err = clReleaseMemObject(srcA);
    if (srcB) err = clReleaseMemObject(srcB);
    if (dstMem) err = clReleaseMemObject(dstMem);
    if (commandQueue) err = clReleaseCommandQueue(commandQueue);
    if (device) err = clReleaseDevice(device);
    if (context) err = clReleaseContext(context);
}
#pragma endregion

#pragma region device check boilerplate
cl_platform_id FindOpenCLPlatform(const char* preferredPlatform, cl_device_type deviceType)
{
    cl_uint numPlatforms = 0;

    clGetPlatformIDs(0, NULL, &numPlatforms);

    if (0 == numPlatforms) return NULL;

    std::vector<cl_platform_id> platforms(numPlatforms);

    clGetPlatformIDs(numPlatforms, &platforms[0], NULL);

    for (cl_uint i = 0; i < numPlatforms; i++)
    {
        bool match = true;
        cl_uint numDevices = 0;

        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0)) match = true;

        if (match)
        {
            clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);

            if (0 != numDevices) return platforms[i];
        }
    }

    return NULL;
}
#pragma endregion

// inputs 
void generateInput(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight)
{
    srand(20);

    // initialise data
    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i)
    {
        switch (i % 4) {
        case 0:
            // the cell has an initial velocity 0, -1 or 1 in x direction
            if (rand() % 2 == 0) inputArray[i] = rand() % 2;
            else inputArray[i] = (int)(-1 * rand() % 2);
            break;
        case 1:
            // the cell has an initial velocity 0, -1 or 1 in y direction
            if (rand() % 2 == 0) inputArray[i] = rand() % 2;
            else inputArray[i] = (int)(-1 * rand() % 2);
            break;
        case 2:
            // the cell has a weignt between 1 and 4
            inputArray[i] = 1 + rand() % 3;
            break;
        case 3:
            // the cell is either alive or dead
            inputArray[i] = rand() % 2;
            break;
        }
    }

    // calculate averages, input into padded matrix
    // ... 

    // check initialisation
    for (int i = 0; i < arrayHeight; i++) {
        for (int j = 0; j < arrayWidth; j++) {
            if (j % 4 == 0) printf("( ");
            printf("%d ", inputArray[4 * i + j]);
            if (j % 4 == 3) printf(" )");
        }
        printf("\n");
    }

}

void matrixPass(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight)
{
    srand(20);

    // use hardcoded 2 degree neibours
    // ... 

    // use padded elments to calculate iteration
    // ...

    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i)
    {
        int x = (int)(i % arrayWidth);
        int y = (int)(i / arrayWidth);

        inputArray[i] = 0;

    }
    printf("\n\n");
}

#pragma region program and kernel creation
int ReadSourceFromFile(const char* fileName, char** source, size_t* sourceSize)
{
    int errorCode = CL_SUCCESS;

    FILE* fp = NULL;
    fopen_s(&fp, fileName, "rb");
    if (!fp == NULL)
    {
        fseek(fp, 0, SEEK_END);
        *sourceSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *source = new char[*sourceSize];
        if (*source == NULL)
        {
            errorCode = CL_OUT_OF_HOST_MEMORY;
        }
        else {
            fread(*source, 1, *sourceSize, fp);
        }
    }
    return errorCode;
}

void SetupOpenCL(ocl_args_d_t* ocl, cl_device_type deviceType)
{
    cl_platform_id platformId = FindOpenCLPlatform("NVIDIA", deviceType);
    cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0 };
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, NULL);
    clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion)
    {
        const cl_command_queue_properties properties[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
        ocl->commandQueue = clCreateCommandQueueWithProperties(ocl->context, ocl->device, properties, NULL);
    }
}

void CreateAndBuildProgram(ocl_args_d_t* ocl)
{
    char* source = NULL;
    size_t src_size = 0;

    ReadSourceFromFile("simple.cl", &source, &src_size);
    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, NULL);
    clBuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL);
}

// use this to edit kernel inputs
void CreateBufferArguments(ocl_args_d_t* ocl, cl_int* inputA, cl_int* inputB, cl_int* outputC, cl_uint arrayWidth, cl_uint arrayHeight)
{
    size_t size = arrayWidth * arrayHeight;

    ocl->srcA = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, inputA, NULL);
    ocl->srcB = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, inputB, NULL);
    ocl->dstMem = clCreateBuffer(ocl->context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, size, outputC, NULL);
}

void SetKernelArguments(ocl_args_d_t* ocl)
{
    clSetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void*)&ocl->srcA);
    clSetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void*)&ocl->srcB);
    clSetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void*)&ocl->dstMem);
}

void ExecuteAddKernel(ocl_args_d_t* ocl, cl_uint width, cl_uint height)
{
    size_t globalWorkSize[2] = { width, height };

    clEnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
}
#pragma endregion

// need to us buffer enqueue
void ReadAndVerify(ocl_args_d_t* ocl, cl_uint width, cl_uint height, cl_int* inputA, cl_int* inputB)
{
    // Read the memory buffer C on the device to the local variable C
    int* C_;
    C_ = (int*)malloc(sizeof(int) * width * height);
    clEnqueueReadBuffer(ocl->commandQueue, ocl->dstMem, CL_TRUE, 0, 
        width * height * sizeof(int), C_, 0, NULL, NULL);

    // Display the result to the screen
    for (int i = 0; i < width * height; i++) {
        printf("%d\n", C_[i]);

    }
    
    clFinish(ocl->commandQueue);
}

int _tmain(int argc, TCHAR* argv[])
{
    cl_int err;
    ocl_args_d_t ocl;
    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

    LARGE_INTEGER perfFrequency;
    LARGE_INTEGER performanceCountNDRangeStart;
    LARGE_INTEGER performanceCountNDRangeStop;

    cl_uint arrayWidth = 16;
    cl_uint arrayHeight = sqrt(arrayWidth);

    SetupOpenCL(&ocl, deviceType);

    cl_uint optimizedSize = ((sizeof(cl_int) * arrayWidth * arrayHeight - 1) / 64 + 1) * 64;
    cl_int* inputA = (cl_int*)_aligned_malloc(optimizedSize, 2 * arrayWidth);
    cl_int* inputB = (cl_int*)_aligned_malloc(optimizedSize, 2 * arrayWidth);
    cl_int* outputC = (cl_int*)_aligned_malloc(optimizedSize, 2 * arrayWidth);

    generateInput(inputA, arrayWidth, arrayHeight);
    matrixPass(inputB, arrayWidth, arrayHeight);

    CreateBufferArguments(&ocl, inputA, inputB, outputC, arrayWidth, arrayHeight);

    CreateAndBuildProgram(&ocl);

    ocl.kernel = clCreateKernel(ocl.program, "Add", &err);

    SetKernelArguments(&ocl);

    ExecuteAddKernel(&ocl, arrayWidth, arrayHeight);

    printf("\n");

    ReadAndVerify(&ocl, arrayWidth, arrayHeight, inputA, inputB);

    printf("\n");

    // insert breakpoint here to inspect kernel output
    int bp = 0;

    _aligned_free(inputA);
    _aligned_free(inputB);
    _aligned_free(outputC);

    return 0;
}
