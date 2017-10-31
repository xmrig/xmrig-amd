/*
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>


static inline void port_sleep(size_t sec)
{
    Sleep(sec * 1000);
}
#else
#include <unistd.h>

static inline void port_sleep(size_t sec)
{
    sleep(sec);
}
#endif // _WIN32


#include <algorithm>
#include <cassert>
#include <iostream>
#include <math.h>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <inttypes.h>


#include "amd/OclGPU.h"
#include "cryptonight.h"
#include "log/Log.h"
#include "Options.h"


constexpr const char *kSetKernelArgErr = "Error %s when calling clSetKernelArg for kernel %d, argument %d.";


const char* err_to_str(cl_int ret)
{
    switch(ret)
    {
    case CL_SUCCESS:
        return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND:
        return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE:
        return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE:
        return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES:
        return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY:
        return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP:
        return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE:
        return "CL_BUILD_PROGRAM_FAILURE";
    case CL_MAP_FAILURE:
        return "CL_MAP_FAILURE";
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case CL_COMPILE_PROGRAM_FAILURE:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case CL_LINKER_NOT_AVAILABLE:
        return "CL_LINKER_NOT_AVAILABLE";
    case CL_LINK_PROGRAM_FAILURE:
        return "CL_LINK_PROGRAM_FAILURE";
    case CL_DEVICE_PARTITION_FAILED:
        return "CL_DEVICE_PARTITION_FAILED";
    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
    case CL_INVALID_VALUE:
        return "CL_INVALID_VALUE";
    case CL_INVALID_DEVICE_TYPE:
        return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_PLATFORM:
        return "CL_INVALID_PLATFORM";
    case CL_INVALID_DEVICE:
        return "CL_INVALID_DEVICE";
    case CL_INVALID_CONTEXT:
        return "CL_INVALID_CONTEXT";
    case CL_INVALID_QUEUE_PROPERTIES:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE:
        return "CL_INVALID_COMMAND_QUEUE";
    case CL_INVALID_HOST_PTR:
        return "CL_INVALID_HOST_PTR";
    case CL_INVALID_MEM_OBJECT:
        return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE:
        return "CL_INVALID_IMAGE_SIZE";
    case CL_INVALID_SAMPLER:
        return "CL_INVALID_SAMPLER";
    case CL_INVALID_BINARY:
        return "CL_INVALID_BINARY";
    case CL_INVALID_BUILD_OPTIONS:
        return "CL_INVALID_BUILD_OPTIONS";
    case CL_INVALID_PROGRAM:
        return "CL_INVALID_PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case CL_INVALID_KERNEL_NAME:
        return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_DEFINITION:
        return "CL_INVALID_KERNEL_DEFINITION";
    case CL_INVALID_KERNEL:
        return "CL_INVALID_KERNEL";
    case CL_INVALID_ARG_INDEX:
        return "CL_INVALID_ARG_INDEX";
    case CL_INVALID_ARG_VALUE:
        return "CL_INVALID_ARG_VALUE";
    case CL_INVALID_ARG_SIZE:
        return "CL_INVALID_ARG_SIZE";
    case CL_INVALID_KERNEL_ARGS:
        return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_DIMENSION:
        return "CL_INVALID_WORK_DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case CL_INVALID_WORK_ITEM_SIZE:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case CL_INVALID_GLOBAL_OFFSET:
        return "CL_INVALID_GLOBAL_OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case CL_INVALID_EVENT:
        return "CL_INVALID_EVENT";
    case CL_INVALID_OPERATION:
        return "CL_INVALID_OPERATION";
    case CL_INVALID_GL_OBJECT:
        return "CL_INVALID_GL_OBJECT";
    case CL_INVALID_BUFFER_SIZE:
        return "CL_INVALID_BUFFER_SIZE";
    case CL_INVALID_MIP_LEVEL:
        return "CL_INVALID_MIP_LEVEL";
    case CL_INVALID_GLOBAL_WORK_SIZE:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case CL_INVALID_PROPERTY:
        return "CL_INVALID_PROPERTY";
    case CL_INVALID_IMAGE_DESCRIPTOR:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case CL_INVALID_COMPILER_OPTIONS:
        return "CL_INVALID_COMPILER_OPTIONS";
    case CL_INVALID_LINKER_OPTIONS:
        return "CL_INVALID_LINKER_OPTIONS";
    case CL_INVALID_DEVICE_PARTITION_COUNT:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";
#ifdef CL_VERSION_2_0
    case CL_INVALID_PIPE_SIZE:
        return "CL_INVALID_PIPE_SIZE";
    case CL_INVALID_DEVICE_QUEUE:
        return "CL_INVALID_DEVICE_QUEUE";
#endif
    default:
        return "UNKNOWN_ERROR";
    }
}


inline static int getDeviceMaxComputeUnits(cl_device_id id)
{
    int count = 0;
    clGetDeviceInfo(id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(int), &count, nullptr);

    return count;
}


inline static void getDeviceName(cl_device_id id, char *buf, size_t size)
{
    if (clGetDeviceInfo(id, 0x4038 /* CL_DEVICE_BOARD_NAME_AMD */, size, buf, nullptr) == CL_SUCCESS) {
        return;
    }

    clGetDeviceInfo(id, CL_DEVICE_NAME, size, buf, nullptr);
}


inline static bool setKernelArgFromExtraBuffers(GpuContext *ctx, size_t kernel, cl_uint argument, size_t offset)
{
    cl_int ret;
    if ((ret = clSetKernelArg(ctx->Kernels[kernel], argument, sizeof(cl_mem), ctx->ExtraBuffers + offset)) != CL_SUCCESS)
    {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), kernel, argument);
        return false;
    }

    return true;
}


size_t InitOpenCLGpu(int index, cl_context opencl_ctx, GpuContext* ctx, const char* source_code)
{
    size_t MaximumWorkSize;
    cl_int ret;

    if ((ret = clGetDeviceInfo(ctx->DeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaximumWorkSize, NULL)) != CL_SUCCESS) {
        LOG_ERR("Error %s when querying a device's max worksize using clGetDeviceInfo.", err_to_str(ret));
        return OCL_ERR_API;
    }

    char buf[128] = { 0 };
    getDeviceName(ctx->DeviceID, buf, sizeof(buf));
    ctx->computeUnits = getDeviceMaxComputeUnits(ctx->DeviceID);

    LOG_INFO(Options::i()->colors() ? "\x1B[01;37m#%d\x1B[0m, GPU \x1B[01;37m#%zu\x1B[0m \x1B[01;32m%s\x1B[0m, intensity: \x1B[01;37m%zu\x1B[0m (%zu/%zu), cu: \x1B[01;37m%d"  : "#%d, GPU #%zu (%s), intensity: %zu (%zu/%zu), cu: %d",
        index, ctx->deviceIdx, buf, ctx->rawIntensity, ctx->workSize, MaximumWorkSize, ctx->computeUnits);

#   ifdef CL_VERSION_2_0
    const cl_queue_properties CommandQueueProperties[] = { 0, 0, 0 };
    ctx->CommandQueues = clCreateCommandQueueWithProperties(opencl_ctx, ctx->DeviceID, CommandQueueProperties, &ret);
#   else
    const cl_command_queue_properties CommandQueueProperties = { 0 };
    ctx->CommandQueues = clCreateCommandQueue(opencl_ctx, ctx->DeviceID, CommandQueueProperties, &ret);
#   endif

    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateCommandQueueWithProperties.", err_to_str(ret));
        return OCL_ERR_API;
    }

    ctx->InputBuffer = clCreateBuffer(opencl_ctx, CL_MEM_READ_ONLY, 88, NULL, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create input buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    size_t hashMemSize;
    int threadMemMask;
    int hasIterations;

    if (Options::i()->algo() == Options::ALGO_CRYPTONIGHT) {
        hashMemSize   = MONERO_MEMORY;
        threadMemMask = MONERO_MASK;
        hasIterations = MONERO_ITER;
    } else {
        hashMemSize   = AEON_MEMORY;
        threadMemMask = AEON_MASK;
        hasIterations = AEON_ITER;
    }

    size_t g_thd = ctx->rawIntensity;
    ctx->ExtraBuffers[0] = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, hashMemSize * g_thd, NULL, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create hash scratchpads buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    ctx->ExtraBuffers[1] = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, 200 * g_thd, NULL, &ret);
    if(ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create hash states buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Blake-256 branches
    ctx->ExtraBuffers[2] = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), NULL, &ret);
    if (ret != CL_SUCCESS){
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 0 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Groestl-256 branches
    ctx->ExtraBuffers[3] = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), NULL, &ret);
    if(ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 1 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // JH-256 branches
    ctx->ExtraBuffers[4] = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), NULL, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 2 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Skein-512 branches
    ctx->ExtraBuffers[5] = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), NULL, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 3 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Assume we may find up to 0xFF nonces in one run - it's reasonable
    ctx->OutputBuffer = clCreateBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * 0x100, NULL, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create output buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    ctx->Program = clCreateProgramWithSource(opencl_ctx, 1, (const char**)&source_code, NULL, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateProgramWithSource on the contents of cryptonight.cl", err_to_str(ret));
        return OCL_ERR_API;
    }

    char options[256];
    snprintf(options, sizeof(options), "-DITERATIONS=%d -DMASK=%d -DWORKSIZE=%zu", hasIterations, threadMemMask, ctx->workSize);
    ret = clBuildProgram(ctx->Program, 1, &ctx->DeviceID, options, NULL, NULL);
    if (ret != CL_SUCCESS) {
        size_t len;
        LOG_ERR("Error %s when calling clBuildProgram.", err_to_str(ret));

        if ((ret = clGetProgramBuildInfo(ctx->Program, ctx->DeviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &len)) != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clGetProgramBuildInfo for length of build log output.", err_to_str(ret));
            return OCL_ERR_API;
        }

        char* BuildLog = (char*)malloc(len + 1);
        BuildLog[0] = '\0';

        if ((ret = clGetProgramBuildInfo(ctx->Program, ctx->DeviceID, CL_PROGRAM_BUILD_LOG, len, BuildLog, NULL)) != CL_SUCCESS) {
            free(BuildLog);
            LOG_ERR("Error %s when calling clGetProgramBuildInfo for build log.", err_to_str(ret));
            return OCL_ERR_API;
        }
        
        Log::i()->text("Build log:");
        std::cerr << BuildLog << std::endl;

        free(BuildLog);
        return OCL_ERR_API;
    }

    cl_build_status status;
    do
    {
        if ((ret = clGetProgramBuildInfo(ctx->Program, ctx->DeviceID, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL)) != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clGetProgramBuildInfo for status of build.", err_to_str(ret));
            return OCL_ERR_API;
        }
        port_sleep(1);
    }
    while(status == CL_BUILD_IN_PROGRESS);

    const char *KernelNames[] = { "cn0", "cn1", "cn2", "Blake", "Groestl", "JH", "Skein" };
    for(int i = 0; i < 7; ++i) {
        ctx->Kernels[i] = clCreateKernel(ctx->Program, KernelNames[i], &ret);
        if (ret != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clCreateKernel for kernel %s.", err_to_str(ret), KernelNames[i]);
            return OCL_ERR_API;
        }
    }

    ctx->Nonce = 0;
    return 0;
}


cl_uint getNumPlatforms()
{
    cl_uint count = 0;
    cl_int ret;

    if ((ret = clGetPlatformIDs(0, nullptr, &count)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetPlatformIDs for number of platforms.", err_to_str(ret));
    }

    if (count == 0) {
        LOG_ERR("No OpenCL platform found.");
    }

    return count;
}


std::vector<GpuContext> getAMDDevices(int index)
{
    const uint32_t numPlatforms = getNumPlatforms();

    cl_platform_id *platforms = new cl_platform_id[numPlatforms];
    clGetPlatformIDs(numPlatforms, platforms, nullptr);

    cl_uint num_devices;
    clGetDeviceIDs(platforms[index], CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);

    cl_device_id *device_list = new cl_device_id[num_devices];
    clGetDeviceIDs(platforms[index], CL_DEVICE_TYPE_GPU, num_devices, device_list, nullptr);

    std::vector<GpuContext> ctxVec;
    char buf[256] = { 0 };

    for (cl_uint i = 0; i < num_devices; i++) {
        clGetDeviceInfo(device_list[i], CL_DEVICE_VENDOR, sizeof(buf), buf, nullptr);
        if (strstr(buf, "Advanced Micro Devices") == nullptr) {
            continue;
        }

        GpuContext ctx;
        ctx.deviceIdx = i;
        ctx.DeviceID = device_list[i];
        ctx.computeUnits = getDeviceMaxComputeUnits(ctx.DeviceID);

        size_t maxMem;
        clGetDeviceInfo(ctx.DeviceID, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(size_t), &(maxMem), nullptr);
        clGetDeviceInfo(ctx.DeviceID, CL_DEVICE_GLOBAL_MEM_SIZE,    sizeof(size_t), &(ctx.freeMem), nullptr);
        // if environment variable GPU_SINGLE_ALLOC_PERCENT is not set we can not allocate the full memory
        ctx.freeMem = std::min(ctx.freeMem, maxMem);

        getDeviceName(ctx.DeviceID, buf, sizeof(buf));

        LOG_INFO(Options::i()->colors() ? "\x1B[01;32mfound\x1B[0m OpenCL GPU: \x1B[01;37m%s\x1B[0m, cu: \x1B[01;37m%d" : "found OpenCL GPU: %s, cu:", buf, ctx.computeUnits);

        clGetDeviceInfo(ctx.DeviceID, CL_DEVICE_NAME, sizeof(buf), buf, nullptr);
        ctx.name = buf;

        ctxVec.push_back(ctx);
    }
    

    delete [] device_list;
    delete [] platforms;

    return ctxVec;
}


void printPlatforms()
{
    const uint32_t numPlatforms = getNumPlatforms();
    if (numPlatforms == 0) {
        return;
    }

    cl_platform_id *platforms = new cl_platform_id[numPlatforms];
    clGetPlatformIDs(numPlatforms, platforms, nullptr);

    char buf[128] = { 0 };

    for (int i = 0; i < numPlatforms; i++) {
        if (clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buf), buf, nullptr) != CL_SUCCESS) {
            continue;
        }

        printf("#%d: %s\n", i, buf);
    }

    delete[] platforms;
}


int getAMDPlatformIdx()
{
    const uint32_t numPlatforms = getNumPlatforms();
    if (numPlatforms == 0) {
        return -1;
    }

#   if !defined(__APPLE__)
    cl_platform_id *platforms = new cl_platform_id[numPlatforms];
    clGetPlatformIDs(numPlatforms, platforms, nullptr);

    int platformIndex = -1;
    char buf[256] = { 0 };

    for (int i = 0; i < numPlatforms; i++) {
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buf), buf, nullptr);

        if (strstr(buf, "Advanced Micro Devices") != nullptr) {
            platformIndex = i;
            LOG_INFO(Options::i()->colors() ? "\x1B[01;32mfound\x1B[0m AMD platform index: \x1B[01;37m%d\x1B[0m, name: \x1B[01;37m%s" : "found AMD platform index: %d, name: %s", i , buf);
            break;
        }
    }

    delete [] platforms;
    return platformIndex;
#   else
    return 0;
#   endif
}


// RequestedDeviceIdxs is a list of OpenCL device indexes
// NumDevicesRequested is number of devices in RequestedDeviceIdxs list
// Returns 0 on success, -1 on stupid params, -2 on OpenCL API error
size_t InitOpenCL(GpuContext* ctx, size_t num_gpus, size_t platform_idx)
{
    cl_uint entries = getNumPlatforms();
    if (entries == 0) {
        return OCL_ERR_API;
    }

    // The number of platforms naturally is the index of the last platform plus one.
    if (entries <= platform_idx) {
        LOG_ERR("Selected OpenCL platform index %d doesn't exist.", platform_idx);
        return OCL_ERR_BAD_PARAMS;
    }

    cl_platform_id *platforms = new cl_platform_id[entries];
    clGetPlatformIDs(entries, platforms, nullptr);

    char buf[256] = { 0 };
    clGetPlatformInfo(platforms[platform_idx], CL_PLATFORM_VENDOR, sizeof(buf), buf, nullptr);

    if (strstr(buf, "Advanced Micro Devices") == nullptr) {
        LOG_WARN("using non AMD device: %s", buf);
    }

    delete [] platforms;

    /*MSVC skimping on devel costs by shoehorning C99 to be a subset of C++? Noooo... can't be.*/
#   ifdef __GNUC__
    cl_platform_id PlatformIDList[entries];
#   else
    cl_platform_id* PlatformIDList = (cl_platform_id*)_alloca(entries * sizeof(cl_platform_id));
#   endif

    cl_int ret;
    if ((ret = clGetPlatformIDs(entries, PlatformIDList, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetPlatformIDs for platform ID information.", err_to_str(ret));
        return OCL_ERR_API;
    }

    if ((ret = clGetDeviceIDs(PlatformIDList[platform_idx], CL_DEVICE_TYPE_GPU, 0, nullptr, &entries)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetDeviceIDs for number of devices.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Same as the platform index sanity check, except we must check all requested device indexes
    for (int i = 0; i < num_gpus; ++i) {
        if (entries <= ctx[i].deviceIdx) {
            LOG_ERR("Selected OpenCL device index %lu doesn't exist.\n", ctx[i].deviceIdx);
            return OCL_ERR_BAD_PARAMS;
        }
    }

#   ifdef __GNUC__
    cl_device_id DeviceIDList[entries];
#   else
    cl_device_id* DeviceIDList = (cl_device_id*)_alloca(entries * sizeof(cl_device_id));
#   endif

    if((ret = clGetDeviceIDs(PlatformIDList[platform_idx], CL_DEVICE_TYPE_GPU, entries, DeviceIDList, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetDeviceIDs for device ID information.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Indexes sanity checked above
#   ifdef __GNUC__
    cl_device_id TempDeviceList[num_gpus];
#   else
    cl_device_id* TempDeviceList = (cl_device_id*)_alloca(entries * sizeof(cl_device_id));
#   endif

    for (int i = 0; i < num_gpus; ++i) {
        ctx[i].DeviceID = DeviceIDList[ctx[i].deviceIdx];
        TempDeviceList[i] = DeviceIDList[ctx[i].deviceIdx];
    }

    cl_context opencl_ctx = clCreateContext(nullptr, num_gpus, TempDeviceList, nullptr, nullptr, &ret);
    if(ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateContext.", err_to_str(ret));
        return OCL_ERR_API;
    }

    const char *cryptonightCL =
            #include "./opencl/cryptonight.cl"
    ;
    const char *blake256CL =
            #include "./opencl/blake256.cl"
    ;
    const char *groestl256CL =
            #include "./opencl/groestl256.cl"
    ;
    const char *jhCL =
            #include "./opencl/jh.cl"
    ;
    const char *wolfAesCL =
            #include "./opencl/wolf-aes.cl"
    ;
    const char *wolfSkeinCL =
            #include "./opencl/wolf-skein.cl"
    ;

    std::string source_code(cryptonightCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_WOLF_AES"), wolfAesCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_WOLF_SKEIN"), wolfSkeinCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_JH"), jhCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_BLAKE256"), blake256CL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_GROESTL256"), groestl256CL);

    for (int i = 0; i < num_gpus; ++i) {
        if ((ret = InitOpenCLGpu(i, opencl_ctx, &ctx[i], source_code.c_str())) != OCL_ERR_SUCCESS) {
            return ret;
        }
    }

    return OCL_ERR_SUCCESS;
}

size_t XMRSetJob(GpuContext* ctx, uint8_t* input, size_t input_len, uint64_t target)
{
    cl_int ret;

    if (input_len > 84) {
        return OCL_ERR_BAD_PARAMS;
    }

    input[input_len] = 0x01;
    memset(input + input_len + 1, 0, 88 - input_len - 1);
    
    size_t numThreads = ctx->rawIntensity;

    if ((ret = clEnqueueWriteBuffer(ctx->CommandQueues, ctx->InputBuffer, CL_TRUE, 0, 88, input, 0, NULL, NULL)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueWriteBuffer to fill input buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    if ((ret = clSetKernelArg(ctx->Kernels[0], 0, sizeof(cl_mem), &ctx->InputBuffer)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 0, 0);
        return OCL_ERR_API;
    }

    // Scratchpads, States
    if (!setKernelArgFromExtraBuffers(ctx, 0, 1, 0) || !setKernelArgFromExtraBuffers(ctx, 0, 2, 1)) {
        return OCL_ERR_API;
    }

    // Threads
    if((ret = clSetKernelArg(ctx->Kernels[0], 3, sizeof(cl_ulong), &numThreads)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 0, 3);
        return OCL_ERR_API;
    }

    // CN2 Kernel
    // Scratchpads, States
    if (!setKernelArgFromExtraBuffers(ctx, 1, 0, 0) || !setKernelArgFromExtraBuffers(ctx, 1, 1, 1)) {
        return OCL_ERR_API;
    }

    // Threads
    if ((ret = clSetKernelArg(ctx->Kernels[1], 2, sizeof(cl_ulong), &numThreads)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 1, 2);
        return(OCL_ERR_API);
    }

    // CN3 Kernel
    // Scratchpads, States
    if (!setKernelArgFromExtraBuffers(ctx, 2, 0, 0) || !setKernelArgFromExtraBuffers(ctx, 2, 1, 1)) {
        return OCL_ERR_API;
    }

    // Branch 0-3
    for (size_t i = 0; i < 4; ++i) {
        if (!setKernelArgFromExtraBuffers(ctx, 2, i + 2, i + 2)) {
            return OCL_ERR_API;
        }
    }

    // Threads
    if((ret = clSetKernelArg(ctx->Kernels[2], 6, sizeof(cl_ulong), &numThreads)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 2, 6);
        return OCL_ERR_API;
    }

    for (int i = 0; i < 4; ++i) {
        // Nonce buffer, Output
        if (!setKernelArgFromExtraBuffers(ctx, i + 3, 0, 1) || !setKernelArgFromExtraBuffers(ctx, i + 3, 1, i + 2)) {
            return OCL_ERR_API;
        }

        // Output
        if ((ret = clSetKernelArg(ctx->Kernels[i + 3], 2, sizeof(cl_mem), &ctx->OutputBuffer)) != CL_SUCCESS) {
            LOG_ERR(kSetKernelArgErr, err_to_str(ret), i + 3, 2);
            return OCL_ERR_API;
        }

        // Target
        if ((ret = clSetKernelArg(ctx->Kernels[i + 3], 3, sizeof(cl_ulong), &target)) != CL_SUCCESS) {
            LOG_ERR(kSetKernelArgErr, err_to_str(ret), i + 3, 3);
            return OCL_ERR_API;
        }
    }

    return OCL_ERR_SUCCESS;
}

size_t XMRRunJob(GpuContext* ctx, cl_uint* HashOutput)
{
    cl_int ret;
    cl_uint zero = 0;
    size_t BranchNonces[4];
    memset(BranchNonces,0,sizeof(size_t)*4);

    size_t g_intensity = ctx->rawIntensity;
    size_t w_size = ctx->workSize;
    // round up to next multiple of w_size
    size_t g_thd = ((g_intensity + w_size - 1u) / w_size) * w_size;
    // number of global threads must be a multiple of the work group size (w_size)
    assert(g_thd%w_size == 0);

    for(int i = 2; i < 6; ++i)
    {
        if((ret = clEnqueueWriteBuffer(ctx->CommandQueues, ctx->ExtraBuffers[i], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), &zero, 0, NULL, NULL)) != CL_SUCCESS)
        {
            LOG_ERR("Error %s when calling clEnqueueWriteBuffer to zero branch buffer counter %d.", err_to_str(ret), i - 2);
            return OCL_ERR_API;
        }
    }

    if((ret = clEnqueueWriteBuffer(ctx->CommandQueues, ctx->OutputBuffer, CL_FALSE, sizeof(cl_uint) * 0xFF, sizeof(cl_uint), &zero, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueReadBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    clFinish(ctx->CommandQueues);

    size_t Nonce[2] = {ctx->Nonce, 1}, gthreads[2] = { g_thd, 8 }, lthreads[2] = { w_size, 8 };
    if((ret = clEnqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[0], 2, Nonce, gthreads, lthreads, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), 0);
        return OCL_ERR_API;
    }

    /*for(int i = 1; i < 3; ++i)
    {
        if((ret = clEnqueueNDRangeKernel(*ctx->CommandQueues, ctx->Kernels[i], 1, &ctx->Nonce, &g_thd, &w_size, 0, NULL, NULL)) != CL_SUCCESS)
        {
            Log(LOG_CRITICAL, "Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), i);
            return(ERR_OCL_API);
        }
    }*/

    size_t tmpNonce = ctx->Nonce;
    if((ret = clEnqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[1], 1, &tmpNonce, &g_thd, &w_size, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), 1);
        return OCL_ERR_API;
    }

    if((ret = clEnqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[2], 2, Nonce, gthreads, lthreads, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), 2);
        return OCL_ERR_API;
    }

    if((ret = clEnqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[2], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueReadBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    if((ret = clEnqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[3], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces + 1, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueReadBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    if((ret = clEnqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[4], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces + 2, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueReadBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    if((ret = clEnqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[5], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces + 3, 0, NULL, NULL)) != CL_SUCCESS)
    {
        LOG_ERR("Error %s when calling clEnqueueReadBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    clFinish(ctx->CommandQueues);

    for (int i = 0; i < 4; ++i) {
        if (BranchNonces[i]) {
            // Threads
            if ((clSetKernelArg(ctx->Kernels[i + 3], 4, sizeof(cl_ulong), BranchNonces + i)) != CL_SUCCESS) {
                LOG_ERR(kSetKernelArgErr, err_to_str(ret), i + 3, 4);
                return OCL_ERR_API;
            }

            // round up to next multiple of w_size
            BranchNonces[i] = ((BranchNonces[i] + w_size - 1u) / w_size) * w_size;
            // number of global threads must be a multiple of the work group size (w_size)
            assert(BranchNonces[i]%w_size == 0);
            size_t tmpNonce = ctx->Nonce;
            if ((ret = clEnqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[i + 3], 1, &tmpNonce, BranchNonces + i, &w_size, 0, NULL, NULL)) != CL_SUCCESS) {
                LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), i + 3);
                return OCL_ERR_API;
            }
        }
    }

    if ((ret = clEnqueueReadBuffer(ctx->CommandQueues, ctx->OutputBuffer, CL_TRUE, 0, sizeof(cl_uint) * 0x100, HashOutput, 0, NULL, NULL)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueReadBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    clFinish(ctx->CommandQueues);
    auto & numHashValues = HashOutput[0xFF];
    // avoid out of memory read, we have only storage for 0xFF results
    if (numHashValues > 0xFF) {
        numHashValues = 0xFF;
    }

    ctx->Nonce += (uint32_t) g_intensity;

    return OCL_ERR_SUCCESS;
}
