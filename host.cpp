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

#pragma region create / destroy boilerplate
struct ocl_args_d_t
{
    ocl_args_d_t();
    ~ocl_args_d_t();

    // Regular OpenCL objects:
    cl_context       context;           // hold the context handler
    cl_device_id     device;            // hold the selected device handler
    cl_command_queue commandQueue;      // hold the commands-queue handler
    cl_program       program;           // hold the program handler
    cl_kernel        kernel;            // hold the kernel handler
    float            platformVersion;   // hold the OpenCL platform version (default 1.2)
    float            deviceVersion;     // hold the OpenCL device version (default. 1.2)
    float            compilerVersion;   // hold the device OpenCL C version (default. 1.2)

    // Objects that are specific for algorithm implemented in this sample
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
bool CheckPreferredPlatformMatch(cl_platform_id platform, const char* preferredPlatform)
{
    size_t stringLength = 0;
    cl_int err = CL_SUCCESS;
    bool match = false;

    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &stringLength);
    if (CL_SUCCESS != err) return false;

    std::vector<char> platformName(stringLength);

    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, stringLength, &platformName[0], NULL);
    if (CL_SUCCESS != err) return false;

    if (strstr(&platformName[0], preferredPlatform) != 0)
    {
        LogInfo("Platform: %s\n", &platformName[0]);
        match = true;
    }

    return match;
}

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

        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0)) match = CheckPreferredPlatformMatch(platforms[i], preferredPlatform);

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
    if (CL_SUCCESS != err) return err;

    std::vector<char> platformVersion(stringLength);

    err = clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, stringLength, &platformVersion[0], NULL);
    if (CL_SUCCESS != err) return err;

    if (strstr(&platformVersion[0], "OpenCL 2.0") != NULL) ocl->platformVersion = OPENCL_VERSION_2_0;

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, 0, NULL, &stringLength);
    if (CL_SUCCESS != err) return err;

    std::vector<char> deviceVersion(stringLength);

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, stringLength, &deviceVersion[0], NULL);
    if (CL_SUCCESS != err) return err;

    if (strstr(&deviceVersion[0], "OpenCL 2.0") != NULL) ocl->deviceVersion = OPENCL_VERSION_2_0;

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &stringLength);
    if (CL_SUCCESS != err) return err;

    std::vector<char> compilerVersion(stringLength);

    err = clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, stringLength, &compilerVersion[0], NULL);
    if (CL_SUCCESS != err) return err;

    else if (strstr(&compilerVersion[0], "OpenCL C 2.0") != NULL) ocl->compilerVersion = OPENCL_VERSION_2_0;

    return err;
}
#pragma endregion

void generateInput(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight)
{
    srand(20);

    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i)
    {
        inputArray[i] = rand();
    }
}

int SetupOpenCL(ocl_args_d_t *ocl, cl_device_type deviceType)
{
    cl_int err = CL_SUCCESS;

    cl_platform_id platformId = FindOpenCLPlatform("Intel", deviceType);
    if (NULL == platformId) return CL_INVALID_VALUE;

    cl_context_properties contextProperties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0};
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &err);
    if ((CL_SUCCESS != err) || (NULL == ocl->context)) return err;

    err = clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    if (CL_SUCCESS != err) return err;

    GetPlatformAndDeviceVersion(platformId, ocl);

#ifdef CL_VERSION_2_0
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion)
    {
        const cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
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

int CreateAndBuildProgram(ocl_args_d_t *ocl)
{
    cl_int err = CL_SUCCESS;

    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("simple.cl", &source, &src_size);
    if (CL_SUCCESS != err) goto Finish;

    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, &err);
    if (CL_SUCCESS != err) goto Finish;

    err = clBuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL);
    if (CL_SUCCESS != err)
    {

        if (err == CL_BUILD_PROGRAM_FAILURE)
        {
            size_t log_size = 0;
            clGetProgramBuildInfo(ocl->program, ocl->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

            std::vector<char> build_log(log_size);
            clGetProgramBuildInfo(ocl->program, ocl->device, CL_PROGRAM_BUILD_LOG, log_size, &build_log[0], NULL);
        }
    }

Finish:
    if (source)
    {
        delete[] source;
        source = NULL;
    }

    return err;
}

int CreateBufferArguments(ocl_args_d_t *ocl, cl_int* inputA, cl_int* inputB, cl_int* outputC, cl_uint arrayWidth, cl_uint arrayHeight)
{
    cl_int err = CL_SUCCESS;

    cl_image_format format;
    cl_image_desc desc;

    format.image_channel_data_type = CL_UNSIGNED_INT32;
    format.image_channel_order     = CL_R;

    // Define the image properties (descriptor)
    desc.image_type        = CL_MEM_OBJECT_IMAGE2D;
    desc.image_width       = arrayWidth;
    desc.image_height      = arrayHeight;
    desc.image_depth       = 0;
    desc.image_array_size  = 1;
    desc.image_row_pitch   = 0;
    desc.image_slice_pitch = 0;
    desc.num_mip_levels    = 0;
    desc.num_samples       = 0;
#ifdef CL_VERSION_2_0
    desc.mem_object        = NULL;
#else
    desc.buffer            = NULL;
#endif

    // Create first image based on host memory inputA
    ocl->srcA = clCreateImage(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &format, &desc, inputA, &err);
    if (CL_SUCCESS != err) return err;

    // Create second image based on host memory inputB
    ocl->srcB = clCreateImage(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &format, &desc, inputB, &err);
    if (CL_SUCCESS != err) return err;

    // Create third (output) image based on host memory outputC
    ocl->dstMem = clCreateImage(ocl->context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, &format, &desc, outputC, &err);
    if (CL_SUCCESS != err) return err;


    return CL_SUCCESS;
}

cl_uint SetKernelArguments(ocl_args_d_t *ocl)
{
    cl_int err = CL_SUCCESS;

    err  =  clSetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void *)&ocl->srcA);
    if (CL_SUCCESS != err) return err;

    err  = clSetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void *)&ocl->srcB);
    if (CL_SUCCESS != err) return err;

    err  = clSetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void *)&ocl->dstMem);
    if (CL_SUCCESS != err) return err;

    return err;
}

cl_uint ExecuteAddKernel(ocl_args_d_t *ocl, cl_uint width, cl_uint height)
{
    cl_int err = CL_SUCCESS;

    size_t globalWorkSize[2] = {width, height};

    err = clEnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
    if (CL_SUCCESS != err) return err;

    err = clFinish(ocl->commandQueue);
    if (CL_SUCCESS != err) return err;

    return CL_SUCCESS;
}

bool ReadAndVerify(ocl_args_d_t *ocl, cl_uint width, cl_uint height, cl_int *inputA, cl_int *inputB)
{
    cl_int err = CL_SUCCESS;
    bool result = true;

    size_t origin[] = {0, 0, 0};
    size_t region[] = {width, height, 1};
    size_t image_row_pitch;
    size_t image_slice_pitch;
    cl_int *resultPtr = (cl_int *)clEnqueueMapImage(ocl->commandQueue, ocl->dstMem, true, CL_MAP_READ, origin, region, &image_row_pitch, &image_slice_pitch, 0, NULL, NULL, &err);

    if (CL_SUCCESS != err) return false;

    err = clFinish(ocl->commandQueue);

    unsigned int size = width * height;
    for (unsigned int k = 0; k < size; ++k)
    {
        if (resultPtr[k] != inputA[k] + inputB[k]) result = false;
    }

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

    cl_uint arrayWidth  = 4;
    cl_uint arrayHeight = 4;

    SetupOpenCL(&ocl, deviceType);

    cl_uint optimizedSize = ((sizeof(cl_int) * arrayWidth * arrayHeight - 1)/64 + 1) * 64;
    cl_int* inputA  = (cl_int*)_aligned_malloc(optimizedSize, 2 * arrayWidth);
    cl_int* inputB  = (cl_int*)_aligned_malloc(optimizedSize, 2 * arrayWidth);
    cl_int* outputC = (cl_int*)_aligned_malloc(optimizedSize, 2 * arrayWidth);

    generateInput(inputA, arrayWidth, arrayHeight);
    generateInput(inputB, arrayWidth, arrayHeight);

    CreateBufferArguments(&ocl, inputA, inputB, outputC, arrayWidth, arrayHeight);

    CreateAndBuildProgram(&ocl);

    ocl.kernel = clCreateKernel(ocl.program, "Add", &err);

    SetKernelArguments(&ocl);

    ExecuteAddKernel(&ocl, arrayWidth, arrayHeight);

    ReadAndVerify(&ocl, arrayWidth, arrayHeight, inputA, inputB);

    // insert breakpoint here to inspect kernel output
    int bp = 0;

    _aligned_free(inputA);
    _aligned_free(inputB);
    _aligned_free(outputC);

    return 0;
}
