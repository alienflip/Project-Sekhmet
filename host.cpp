#pragma region c++ libs/includes, opencl version defs
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>

#define OPENCL_VERSION_1_2  1.2f
#define OPENCL_VERSION_2_0  2.0f

#define CL_TARGET_OPENCL_VERSION 220

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "CL\cl.h"
#include "utils.h"

#include <Windows.h>
#pragma endregion

#pragma region create / destroy boilerplate
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
    cl_int err = CL_SUCCESS;

    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (CL_SUCCESS != err) return NULL;

    if (0 == numPlatforms) return NULL;

    std::vector<cl_platform_id> platforms(numPlatforms);

    err = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
    if (CL_SUCCESS != err) return NULL;

    for (cl_uint i = 0; i < numPlatforms; i++)
    {
        bool match = true;
        cl_uint numDevices = 0;

        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0)) match = true;

        if (match)
        {
            err = clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);

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

#pragma region program creation
int SetupOpenCL(ocl_args_d_t* ocl, cl_device_type deviceType)
{
    cl_int err = CL_SUCCESS;

    cl_platform_id platformId = FindOpenCLPlatform("NVIDIA", deviceType);
    if (NULL == platformId) return CL_INVALID_VALUE;

    cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0 };
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &err);
    if ((CL_SUCCESS != err) || (NULL == ocl->context)) return err;

    err = clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    if (CL_SUCCESS != err) return err;

#ifdef CL_VERSION_2_0
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion)
    {
        const cl_command_queue_properties properties[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
        ocl->commandQueue = clCreateCommandQueueWithProperties(ocl->context, ocl->device, properties, &err);
    }
    else {
        cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
        ocl->commandQueue = clCreateCommandQueue(ocl->context, ocl->device, properties, &err);
    }
#else
    cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
    ocl->commandQueue = clCreateCommandQueue(ocl->context, ocl->device, properties, &err);
#endif
    if (CL_SUCCESS != err) return err;

    return CL_SUCCESS;
}

int CreateAndBuildProgram(ocl_args_d_t* ocl)
{
    cl_int err = CL_SUCCESS;

    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("simple.cl", &source, &src_size);
    if (CL_SUCCESS != err) goto Finish;

    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, &err);
    if (CL_SUCCESS != err) goto Finish;

    err = clBuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL);

Finish:
    if (source)
    {
        delete[] source;
        source = NULL;
    }

    return err;
}
#pragma endregion

// use this to edit kernel inputs
int CreateBufferArguments(ocl_args_d_t* ocl, cl_int* inputA, cl_int* inputB, cl_int* outputC, cl_uint arrayWidth, cl_uint arrayHeight)
{
    cl_int err = CL_SUCCESS;

    cl_image_format format;
    cl_image_desc desc;

    size_t size = arrayWidth * arrayHeight;

    ocl->srcA = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, inputA, &err);
    if (CL_SUCCESS != err) return err;

    ocl->srcB = clCreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, inputB, &err);
    if (CL_SUCCESS != err) return err;

    ocl->dstMem = clCreateBuffer(ocl->context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, size, outputC, &err);
    if (CL_SUCCESS != err) return err;

    return CL_SUCCESS;
}

#pragma region kernel creation
cl_uint SetKernelArguments(ocl_args_d_t* ocl)
{
    cl_int err = CL_SUCCESS;

    err = clSetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void*)&ocl->srcA);
    if (CL_SUCCESS != err) return err;

    err = clSetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void*)&ocl->srcB);
    if (CL_SUCCESS != err) return err;

    err = clSetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void*)&ocl->dstMem);
    if (CL_SUCCESS != err) return err;

    return err;
}

cl_uint ExecuteAddKernel(ocl_args_d_t* ocl, cl_uint width, cl_uint height)
{
    cl_int err = CL_SUCCESS;

    size_t globalWorkSize[2] = { width, height };

    err = clEnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
    if (CL_SUCCESS != err) return err;

    err = clFinish(ocl->commandQueue);
    if (CL_SUCCESS != err) return err;

    return CL_SUCCESS;
}
#pragma endregion

// outputs
bool ReadAndVerify(ocl_args_d_t* ocl, cl_uint width, cl_uint height, cl_int* inputA, cl_int* inputB)
{
    cl_int err = CL_SUCCESS;
    bool result = true;

    size_t origin[] = { 0, 0, 0 };
    size_t region[] = { width, height, 1 };
    size_t image_row_pitch;
    size_t image_slice_pitch;
    cl_int* resultPtr = (cl_int*)clEnqueueMapImage(ocl->commandQueue, ocl->dstMem, true, CL_MAP_READ, origin, region, &image_row_pitch, &image_slice_pitch, 0, NULL, NULL, &err);

    if (CL_SUCCESS != err) return false;

    err = clFinish(ocl->commandQueue);

    unsigned int size = width * height;
    for (unsigned int k = 0; k < size; ++k)
    {
        if (resultPtr[k] != inputA[k] + inputB[k]) result = false;
    }

    for (int i = 0; i < height; i++) {
        printf("\n");
        for (int j = 0; j < width; j++) {
            if (j % 4 == 0) printf("( ");
            printf("%d ", resultPtr[i * width + j]);
            if (j % 4 == 3) printf(" )");
        }
    }

    // calculate averages, repeat processes
    // ... 

    err = clEnqueueUnmapMemObject(ocl->commandQueue, ocl->dstMem, resultPtr, 0, NULL, NULL);

    return result;
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
