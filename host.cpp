#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>
#include <Windows.h>
#include "CL\cl.h"

#pragma region opencl version defs
#define OPENCL_VERSION_1_2  1.2f
#define OPENCL_VERSION_2_0  2.0f
#define CL_TARGET_OPENCL_VERSION 220
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#pragma endregion

#pragma region opencl object holders
struct ocl_args_d_t
{
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
    dstMem(NULL)
{
}
ocl_args_d_t::~ocl_args_d_t()
{
    cl_int err = CL_SUCCESS;

    if (kernel) err = clReleaseKernel(kernel);
    if (program) err = clReleaseProgram(program);
    if (srcA) err = clReleaseMemObject(srcA);
    if (dstMem) err = clReleaseMemObject(dstMem);
    if (commandQueue) err = clReleaseCommandQueue(commandQueue);
    if (device) err = clReleaseDevice(device);
}
#pragma endregion

#pragma region device boilderplate checks
bool CheckPreferredPlatformMatch(cl_platform_id platform, const char* preferredPlatform)
{
    size_t stringLength = 0;
    cl_int err = CL_SUCCESS;
    bool match = false;

    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &stringLength);

    std::vector<char> platformName(stringLength);

    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, stringLength, &platformName[0], NULL);

    if (strstr(&platformName[0], preferredPlatform) != 0) match = true;

    return match;
}
cl_platform_id FindOpenCLPlatform(const char* preferredPlatform, cl_device_type deviceType)
{
    cl_uint numPlatforms = 0;
    cl_int err = CL_SUCCESS;

    err = clGetPlatformIDs(0, NULL, &numPlatforms);

    if (0 == numPlatforms) return NULL;

    std::vector<cl_platform_id> platforms(numPlatforms);

    err = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);

    for (cl_uint i = 0; i < numPlatforms; i++)
    {
        bool match = true;
        cl_uint numDevices = 0;

        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0))
        {
            match = CheckPreferredPlatformMatch(platforms[i], preferredPlatform);
        }

        if (match)
        {
            err = clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);
            if (0 != numDevices) return platforms[i];
        }
    }

    return NULL;
}
int GetPlatformAndDeviceVersion(cl_platform_id platformId, ocl_args_d_t* ocl)
{
    cl_int err = CL_SUCCESS;

    size_t stringLength = 0;
    err = clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, 0, NULL, &stringLength);

    std::vector<char> platformVersion(stringLength);

    err = clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, stringLength, &platformVersion[0], NULL);

    if (strstr(&platformVersion[0], "OpenCL 2.0") != NULL) ocl->platformVersion = OPENCL_VERSION_2_0;

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, 0, NULL, &stringLength);

    std::vector<char> deviceVersion(stringLength);

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, stringLength, &deviceVersion[0], NULL);

    if (strstr(&deviceVersion[0], "OpenCL 2.0") != NULL) ocl->deviceVersion = OPENCL_VERSION_2_0;

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &stringLength);

    std::vector<char> compilerVersion(stringLength);

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, stringLength, &compilerVersion[0], NULL);

    if (strstr(&compilerVersion[0], "OpenCL C 2.0") != NULL) ocl->compilerVersion = OPENCL_VERSION_2_0;

    return err;
}
#pragma endregion

void generateInput(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight)
{
    srand(12345);

    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i)
    {
        inputArray[i] = rand();
    }
}

#pragma region opencl program creation
int SetupOpenCL(ocl_args_d_t* ocl, cl_device_type deviceType)
{
    cl_int err = CL_SUCCESS;

    cl_platform_id platformId = FindOpenCLPlatform("Intel", deviceType);
    cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0 };
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &err);
    err = clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    GetPlatformAndDeviceVersion(platformId, ocl);
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion)
    {
        const cl_command_queue_properties properties[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
        ocl->commandQueue = clCreateCommandQueueWithProperties(ocl->context, ocl->device, properties, &err);
    }
    else {
        cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
        ocl->commandQueue = clCreateCommandQueue(ocl->context, ocl->device, properties, &err);
    }

    return CL_SUCCESS;
}
int ReadSourceFromFile(const char* fileName, char** source, size_t* sourceSize)
{
    int errorCode = CL_SUCCESS;

    FILE* fp = NULL;
    fopen_s(&fp, fileName, "rb");
    if (fp == NULL)
    {
        errorCode = CL_INVALID_VALUE;
    }
    else {
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
int CreateAndBuildProgram(ocl_args_d_t* ocl)
{
    cl_int err = CL_SUCCESS;
    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("simple.cl", &source, &src_size);
    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, &err);
    err = clBuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL);
    return err;
}
#pragma endregion

#pragma region kernel handling
int CreateBufferArguments(ocl_args_d_t* ocl, cl_int* inputA, cl_int* inputB, cl_int* outputC, cl_uint arrayWidth, cl_uint arrayHeight)
{
    cl_int err = CL_SUCCESS;
    unsigned int size = arrayHeight * arrayWidth;
    ocl->srcA = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, inputA, NULL);
    ocl->srcB = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, inputB, NULL);
    ocl->dstMem = clCreateBuffer(ocl->context, CL_MEM_WRITE_ONLY, arrayHeight * arrayWidth * sizeof(int), NULL, &err);
    return CL_SUCCESS;
}
cl_uint SetKernelArguments(ocl_args_d_t* ocl)
{
    cl_int err = CL_SUCCESS;
    err = clSetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void*)&ocl->srcA);
    err = clSetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void*)&ocl->srcB);
    err = clSetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void*)&ocl->dstMem);
    return err;
}
cl_uint ExecuteAddKernel(ocl_args_d_t* ocl, cl_uint width, cl_uint height)
{
    cl_int err = CL_SUCCESS;

    size_t globalWorkSize[2] = { width, height };

    err = clEnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

    err = clFinish(ocl->commandQueue);

    return CL_SUCCESS;
}
#pragma endregion

bool ReadAndVerify(ocl_args_d_t* ocl, cl_uint width, cl_uint height, cl_int* inputA, cl_int* inputB)
{
    cl_int err = CL_SUCCESS;
    bool result = true;
    unsigned int size = width * height;
    int* C = (int*)malloc(sizeof(int) * size);
    cl_int* resultPtr = (cl_int*)clEnqueueReadBuffer(ocl->commandQueue, ocl->dstMem, CL_TRUE, 0, size * sizeof(int), C, 0, NULL, NULL);
    err = clFinish(ocl->commandQueue);
    for (unsigned int k = 0; k < size; ++k)
    {
        printf("%d\n", C[k]);
    }
    err = clEnqueueUnmapMemObject(ocl->commandQueue, ocl->dstMem, resultPtr, 0, NULL, NULL);
    return result;
}

int _tmain(int argc, TCHAR* argv[])
{
    cl_int err;
    ocl_args_d_t ocl;
    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

    cl_uint arrayWidth = 1024;
    cl_uint arrayHeight = 1024;

    if (CL_SUCCESS != SetupOpenCL(&ocl, deviceType))
    {
        return -1;
    }

    cl_uint optimizedSize = ((sizeof(cl_int) * arrayWidth * arrayHeight - 1) / 64 + 1) * 64;
    cl_int* inputA = (cl_int*)_aligned_malloc(optimizedSize, 4096);
    cl_int* inputB = (cl_int*)_aligned_malloc(optimizedSize, 4096);
    cl_int* outputC = (cl_int*)_aligned_malloc(optimizedSize, 4096);

    generateInput(inputA, arrayWidth, arrayHeight);
    generateInput(inputB, arrayWidth, arrayHeight);

    if (CL_SUCCESS != CreateBufferArguments(&ocl, inputA, inputB, outputC, arrayWidth, arrayHeight))
    {
        return -1;
    }
    if (CL_SUCCESS != CreateAndBuildProgram(&ocl))
    {
        return -1;
    }

    ocl.kernel = clCreateKernel(ocl.program, "Add", &err);

    if (CL_SUCCESS != SetKernelArguments(&ocl))
    {
        return -1;
    }
    if (CL_SUCCESS != ExecuteAddKernel(&ocl, arrayWidth, arrayHeight))
    {
        return -1;
    }

    ReadAndVerify(&ocl, arrayWidth, arrayHeight, inputA, inputB);

    int P = 0;

    _aligned_free(inputA);
    _aligned_free(inputB);
    _aligned_free(outputC);

    return 0;
}
