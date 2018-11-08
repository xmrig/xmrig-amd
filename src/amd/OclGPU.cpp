/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 * Copyright 2018 MoneroOcean      <https://github.com/MoneroOcean>, <support@moneroocean.stream>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <math.h>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <inttypes.h>


#include "amd/OclCache.h"
#include "amd/OclError.h"
#include "amd/OclGPU.h"
#include "amd/OclLib.h"
#include "common/log/Log.h"
#include "core/Config.h"
#include "crypto/CryptoNight_constants.h"
#include "cryptonight.h"


constexpr const char *kSetKernelArgErr = "Error %s when calling clSetKernelArg for kernel %d, argument %d.";


inline static const char *err_to_str(cl_int ret)
{
    return OclError::toString(ret);
}


inline static int getDeviceMaxComputeUnits(cl_device_id id)
{
    int count = 0;
    OclLib::getDeviceInfo(id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(int), &count);

    return count;
}


inline static void getDeviceName(cl_device_id id, char *buf, size_t size)
{
    if (OclLib::getDeviceInfo(id, 0x4038 /* CL_DEVICE_BOARD_NAME_AMD */, size, buf) == CL_SUCCESS) {
        return;
    }

    OclLib::getDeviceInfo(id, CL_DEVICE_NAME, size, buf);
}


inline static bool setKernelArgFromExtraBuffers(GpuContext *ctx, size_t kernel, cl_uint argument, size_t offset)
{
    cl_int ret;
    if ((ret = OclLib::setKernelArg(ctx->Kernels[kernel], argument, sizeof(cl_mem), ctx->ExtraBuffers + offset)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), kernel, argument);
        return false;
    }

    return true;
}


inline static int cnKernelOffset(xmrig::Variant variant)
{
    switch (variant) {
    case xmrig::VARIANT_0:
    case xmrig::VARIANT_XHV:
        return 1;

    case xmrig::VARIANT_1:
    case xmrig::VARIANT_XTL:
    case xmrig::VARIANT_RTO:
        return 7;
        break;

    case xmrig::VARIANT_MSR:
        return 8;

    case xmrig::VARIANT_XAO:
        return 9;

    case xmrig::VARIANT_TUBE:
        return 10;

    case xmrig::VARIANT_2:
        return 11;

    default:
        break;
    }

    assert(false);

    return 0;
}


size_t InitOpenCLGpu(int index, cl_context opencl_ctx, GpuContext* ctx, const char* source_code, xmrig::Config *config)
{
    size_t MaximumWorkSize;
    cl_int ret;

    if (OclLib::getDeviceInfo(ctx->DeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &MaximumWorkSize) != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    char buf[128] = { 0 };
    getDeviceName(ctx->DeviceID, buf, sizeof(buf));
    ctx->computeUnits = getDeviceMaxComputeUnits(ctx->DeviceID);

    LOG_INFO(config->isColors() ? WHITE_BOLD("#%d") ", GPU " WHITE_BOLD("#%zu") " " GREEN_BOLD("%s") ", intensity: " WHITE_BOLD("%zu") " (%zu/%zu), unroll: " WHITE_BOLD("%d") ", cu: " WHITE_BOLD("%d")
                                : "#%d, GPU #%zu (%s), intensity: %zu (%zu/%zu), unroll: %d, cu: %d",
        index, ctx->deviceIdx, buf, ctx->rawIntensity, ctx->workSize, MaximumWorkSize, ctx->unrollFactor, ctx->computeUnits);

    ctx->CommandQueues = OclLib::createCommandQueue(opencl_ctx, ctx->DeviceID, &ret);
    if (ret != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    ctx->InputBuffer = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_ONLY, 88, nullptr, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create input buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    size_t g_thd = ctx->rawIntensity;
    ctx->ExtraBuffers[0] = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, xmrig::cn_select_memory(config->algorithm().algo()) * g_thd, nullptr, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create hash scratchpads buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    ctx->ExtraBuffers[1] = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, 200 * g_thd, nullptr, &ret);
    if(ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create hash states buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Blake-256 branches
    ctx->ExtraBuffers[2] = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), nullptr, &ret);
    if (ret != CL_SUCCESS){
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 0 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Groestl-256 branches
    ctx->ExtraBuffers[3] = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), nullptr, &ret);
    if(ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 1 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // JH-256 branches
    ctx->ExtraBuffers[4] = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), nullptr, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 2 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Skein-512 branches
    ctx->ExtraBuffers[5] = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * (g_thd + 2), nullptr, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create Branch 3 buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Assume we may find up to 0xFF nonces in one run - it's reasonable
    ctx->OutputBuffer = OclLib::createBuffer(opencl_ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * 0x100, nullptr, &ret);
    if (ret != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clCreateBuffer to create output buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    OclCache cache(index, opencl_ctx, ctx, source_code, config);
    if (!cache.load()) {
        return OCL_ERR_API;
    }

    const char *KernelNames[] = { "cn0", "cn1", "cn2", "Blake", "Groestl", "JH", "Skein", "cn1_monero", "cn1_msr", "cn1_xao", "cn1_tube", "cn1_v2_monero"};
    for (int i = 0; i < 12; ++i) {
        ctx->Kernels[i] = OclLib::createKernel(ctx->Program, KernelNames[i], &ret);
        if (ret != CL_SUCCESS) {
            return OCL_ERR_API;
        }
    }

    ctx->Nonce = 0;
    return 0;
}


int OclGPU::findPlatformIdx(xmrig::Config *config)
{
    assert(config->vendor() > xmrig::OCL_VENDOR_MANUAL);

#   if !defined(__APPLE__)
    char name[256]  = { 0 };
    const int index = findPlatformIdx(config->vendor(), name, sizeof name);

    if (index >= 0) {
        LOG_INFO(config->isColors() ? GREEN_BOLD("found ") WHITE_BOLD("%s") " platform index: " WHITE_BOLD("%d") ", name: " WHITE_BOLD("%s")
                                    : "found %s platform index: %d, name: %s",
                 config->vendorName(config->vendor()), index , name);
    }

    return index;
#   else
    return 0;
#   endif
}


std::vector<GpuContext> OclGPU::getDevices(xmrig::Config *config)
{
    const int platformIndex               = config->platformIndex();
    std::vector<cl_platform_id> platforms = OclLib::getPlatformIDs();
    std::vector<GpuContext> ctxVec;

    cl_uint num_devices = 0;
    OclLib::getDeviceIDs(platforms[platformIndex], CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
    if (num_devices == 0) {
        return ctxVec;
    }

    cl_device_id *device_list = new cl_device_id[num_devices];
    OclLib::getDeviceIDs(platforms[platformIndex], CL_DEVICE_TYPE_GPU, num_devices, device_list, nullptr);

    char buf[256] = { 0 };

    for (cl_uint i = 0; i < num_devices; i++) {
        OclLib::getDeviceInfo(device_list[i], CL_DEVICE_VENDOR, sizeof(buf), buf);

        GpuContext ctx;
        ctx.deviceIdx = i;
        ctx.DeviceID  = device_list[i];

        if (strstr(buf, "Advanced Micro Devices") != nullptr || strstr(buf, "AMD") != nullptr) {
            ctx.vendor = xmrig::OCL_VENDOR_AMD;
        }
        else if (strstr(buf, "NVIDIA") != nullptr) {
            ctx.vendor = xmrig::OCL_VENDOR_NVIDIA;
        }
        else if (strstr(buf, "Intel") != nullptr) {
            ctx.vendor = xmrig::OCL_VENDOR_INTEL;
        }
        else {
            LOG_WARN("Unknown device vendor: %s", buf);
            continue;
        }

        ctx.computeUnits = getDeviceMaxComputeUnits(ctx.DeviceID);

        size_t maxMem;
        OclLib::getDeviceInfo(ctx.DeviceID, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(size_t), &(maxMem));
        OclLib::getDeviceInfo(ctx.DeviceID, CL_DEVICE_GLOBAL_MEM_SIZE,    sizeof(size_t), &(ctx.freeMem));
        // if environment variable GPU_SINGLE_ALLOC_PERCENT is not set we can not allocate the full memory
        ctx.freeMem = std::min(ctx.freeMem, maxMem);

        getDeviceName(ctx.DeviceID, buf, sizeof(buf));

        LOG_INFO(config->isColors() ? GREEN_BOLD("found") " OpenCL GPU: " GREEN_BOLD("%s") ", cu: " WHITE_BOLD("%d")
                                    : "found OpenCL GPU: %s, cu:",
                 buf, ctx.computeUnits);

        OclLib::getDeviceInfo(ctx.DeviceID, CL_DEVICE_NAME, sizeof(buf), buf);
        ctx.name = buf;

        ctxVec.push_back(ctx);
    }


    delete [] device_list;

    return ctxVec;
}


int OclGPU::findPlatformIdx(xmrig::OclVendor vendor, char *name, size_t nameSize)
{
#   if !defined(__APPLE__)
    std::vector<cl_platform_id> platforms = OclLib::getPlatformIDs();
    if (platforms.empty()) {
        return -1;
    }

    for (uint32_t i = 0; i < platforms.size(); i++) {
        OclLib::getPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, nameSize, name, nullptr);

        switch (vendor) {
        case xmrig::OCL_VENDOR_AMD:
            if (strstr(name, "Advanced Micro Devices") != nullptr) {
                return i;
            }
            break;

        case xmrig::OCL_VENDOR_NVIDIA:
            if (strstr(name, "NVIDIA") != nullptr) {
                return i;
            }
            break;

        case xmrig::OCL_VENDOR_INTEL:
            if (strstr(name, "Intel") != nullptr) {
                return i;
            }
            break;

        default:
            break;
        }
    }

    return -1;
#   else
    return 0;
#   endif
}


void printPlatforms()
{
    std::vector<cl_platform_id> platforms = OclLib::getPlatformIDs();

    char buf[128] = { 0 };

    for (size_t i = 0; i < platforms.size(); i++) {
        if (OclLib::getPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buf), buf, nullptr) != CL_SUCCESS) {
            continue;
        }

        printf("#%zu: %s\n", i, buf);
    }
}


// RequestedDeviceIdxs is a list of OpenCL device indexes
// NumDevicesRequested is number of devices in RequestedDeviceIdxs list
// Returns 0 on success, -1 on stupid params, -2 on OpenCL API error
size_t InitOpenCL(GpuContext* ctx, size_t num_gpus, xmrig::Config *config, cl_context *opencl_ctx)
{
    const size_t platform_idx             = config->platformIndex();
    std::vector<cl_platform_id> platforms = OclLib::getPlatformIDs();
    cl_uint entries                       = platforms.size();

    if (entries == 0) {
        return OCL_ERR_API;
    }

    if (entries <= platform_idx) {
        return OCL_ERR_BAD_PARAMS;
    }

    cl_int ret;
    if ((ret = OclLib::getDeviceIDs(platforms[platform_idx], CL_DEVICE_TYPE_GPU, 0, nullptr, &entries)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetDeviceIDs for number of devices.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Same as the platform index sanity check, except we must check all requested device indexes
    for (size_t i = 0; i < num_gpus; ++i) {
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

    if ((ret = OclLib::getDeviceIDs(platforms[platform_idx], CL_DEVICE_TYPE_GPU, entries, DeviceIDList, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetDeviceIDs for device ID information.", err_to_str(ret));
        return OCL_ERR_API;
    }

    // Indexes sanity checked above
#   ifdef __GNUC__
    cl_device_id TempDeviceList[num_gpus];
#   else
    cl_device_id* TempDeviceList = (cl_device_id*)_alloca(num_gpus * sizeof(cl_device_id));
#   endif

    for (size_t i = 0; i < num_gpus; ++i) {
        ctx[i].DeviceID = DeviceIDList[ctx[i].deviceIdx];
        TempDeviceList[i] = DeviceIDList[ctx[i].deviceIdx];
    }

    // we store previous OpenCL context in static variable to be able to release it next time we do algo switch
    if (*opencl_ctx) OclLib::releaseContext(*opencl_ctx);
    *opencl_ctx = OclLib::createContext(nullptr, num_gpus, TempDeviceList, nullptr, nullptr, &ret);

    if (ret != CL_SUCCESS) {
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
    const char *fastIntMathV2CL =
        #include "./opencl/fast_int_math_v2.cl"
    ;
    const char *fastDivHeavyCL =
        #include "./opencl/fast_div_heavy.cl"
    ;

    std::string source_code(cryptonightCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_WOLF_AES"),         wolfAesCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_WOLF_SKEIN"),       wolfSkeinCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_JH"),               jhCL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_BLAKE256"),         blake256CL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_GROESTL256"),       groestl256CL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_FAST_INT_MATH_V2"), fastIntMathV2CL);
    source_code = std::regex_replace(source_code, std::regex("XMRIG_INCLUDE_FAST_DIV_HEAVY"), fastDivHeavyCL);

    for (size_t i = 0; i < num_gpus; ++i) {
        if (ctx[i].stridedIndex == 2 && (ctx[i].rawIntensity % ctx[i].workSize) != 0) {
            const size_t reduced_intensity = (ctx[i].rawIntensity / ctx[i].workSize) * ctx[i].workSize;
            ctx[i].rawIntensity = reduced_intensity;

            LOG_WARN("AMD GPU #%zu: intensity is not a multiple of 'worksize', auto reduce intensity to %zu", ctx[i].deviceIdx, reduced_intensity);
        }

        if ((ret = InitOpenCLGpu(i, *opencl_ctx, &ctx[i], source_code.c_str(), config)) != OCL_ERR_SUCCESS) {
            return ret;
        }
    }

    return OCL_ERR_SUCCESS;
}

size_t XMRSetJob(GpuContext *ctx, uint8_t *input, size_t input_len, uint64_t target, xmrig::Variant variant)
{
    cl_int ret;

    if (input_len > 84) {
        return OCL_ERR_BAD_PARAMS;
    }

    input[input_len] = 0x01;
    memset(input + input_len + 1, 0, 88 - input_len - 1);
    
    size_t numThreads = ctx->rawIntensity;

    if ((ret = OclLib::enqueueWriteBuffer(ctx->CommandQueues, ctx->InputBuffer, CL_TRUE, 0, 88, input, 0, nullptr, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueWriteBuffer to fill input buffer.", err_to_str(ret));
        return OCL_ERR_API;
    }

    if ((ret = OclLib::setKernelArg(ctx->Kernels[0], 0, sizeof(cl_mem), &ctx->InputBuffer)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 0, 0);
        return OCL_ERR_API;
    }

    // Scratchpads, States
    if (!setKernelArgFromExtraBuffers(ctx, 0, 1, 0) || !setKernelArgFromExtraBuffers(ctx, 0, 2, 1)) {
        return OCL_ERR_API;
    }

    // Threads
    if((ret = OclLib::setKernelArg(ctx->Kernels[0], 3, sizeof(cl_ulong), &numThreads)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 0, 3);
        return OCL_ERR_API;
    }

    // CN1 Kernel
    const int cn_kernel_offset = cnKernelOffset(variant);

    // Scratchpads, States
    if (!setKernelArgFromExtraBuffers(ctx, cn_kernel_offset, 0, 0) || !setKernelArgFromExtraBuffers(ctx, cn_kernel_offset, 1, 1)) {
        return OCL_ERR_API;
    }

    // Threads
    if ((ret = OclLib::setKernelArg(ctx->Kernels[cn_kernel_offset], 2, sizeof(cl_ulong), &numThreads)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 1, 2);
        return(OCL_ERR_API);
    }

    // variant
    const cl_uint v = static_cast<cl_uint>(variant);
    if ((ret = OclLib::setKernelArg(ctx->Kernels[cn_kernel_offset], 3, sizeof(cl_uint), &v)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), cn_kernel_offset, 3);
        return OCL_ERR_API;
    }

    // input
    if ((ret = OclLib::setKernelArg(ctx->Kernels[cn_kernel_offset], 4, sizeof(cl_mem), &ctx->InputBuffer)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), cn_kernel_offset, 4);
        return OCL_ERR_API;
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
    if((ret = OclLib::setKernelArg(ctx->Kernels[2], 6, sizeof(cl_ulong), &numThreads)) != CL_SUCCESS) {
        LOG_ERR(kSetKernelArgErr, err_to_str(ret), 2, 6);
        return OCL_ERR_API;
    }

    for (int i = 0; i < 4; ++i) {
        // Nonce buffer, Output
        if (!setKernelArgFromExtraBuffers(ctx, i + 3, 0, 1) || !setKernelArgFromExtraBuffers(ctx, i + 3, 1, i + 2)) {
            return OCL_ERR_API;
        }

        // Output
        if ((ret = OclLib::setKernelArg(ctx->Kernels[i + 3], 2, sizeof(cl_mem), &ctx->OutputBuffer)) != CL_SUCCESS) {
            LOG_ERR(kSetKernelArgErr, err_to_str(ret), i + 3, 2);
            return OCL_ERR_API;
        }

        // Target
        if ((ret = OclLib::setKernelArg(ctx->Kernels[i + 3], 3, sizeof(cl_ulong), &target)) != CL_SUCCESS) {
            LOG_ERR(kSetKernelArgErr, err_to_str(ret), i + 3, 3);
            return OCL_ERR_API;
        }
    }

    return OCL_ERR_SUCCESS;
}

size_t XMRRunJob(GpuContext *ctx, cl_uint *HashOutput, xmrig::Variant variant)
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
    assert(g_thd % w_size == 0);

    for(int i = 2; i < 6; ++i) {
        if ((ret = OclLib::enqueueWriteBuffer(ctx->CommandQueues, ctx->ExtraBuffers[i], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), &zero, 0, nullptr, nullptr)) != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clEnqueueWriteBuffer to zero branch buffer counter %d.", err_to_str(ret), i - 2);
            return OCL_ERR_API;
        }
    }

    if ((ret = OclLib::enqueueWriteBuffer(ctx->CommandQueues, ctx->OutputBuffer, CL_FALSE, sizeof(cl_uint) * 0xFF, sizeof(cl_uint), &zero, 0, nullptr, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueWriteBuffer to fetch results.", err_to_str(ret));
        return OCL_ERR_API;
    }

    OclLib::finish(ctx->CommandQueues);

    size_t Nonce[2] = {ctx->Nonce, 1}, gthreads[2] = { g_thd, 8 }, lthreads[2] = { w_size, 8 };
    if ((ret = OclLib::enqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[0], 2, Nonce, gthreads, lthreads, 0, nullptr, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), 0);
        return OCL_ERR_API;
    }

    size_t tmpNonce = ctx->Nonce;
    const int cn_kernel_offset = cnKernelOffset(variant);

    if ((ret = OclLib::enqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[cn_kernel_offset], 1, &tmpNonce, &g_thd, &w_size, 0, nullptr, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), 1);
        return OCL_ERR_API;
    }

    if ((ret = OclLib::enqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[2], 2, Nonce, gthreads, lthreads, 0, nullptr, nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), 2);
        return OCL_ERR_API;
    }

    if (OclLib::enqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[2], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces, 0, nullptr, nullptr) != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    if (OclLib::enqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[3], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces + 1, 0, nullptr, nullptr) != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    if (OclLib::enqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[4], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces + 2, 0, nullptr, nullptr) != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    if (OclLib::enqueueReadBuffer(ctx->CommandQueues, ctx->ExtraBuffers[5], CL_FALSE, sizeof(cl_uint) * g_intensity, sizeof(cl_uint), BranchNonces + 3, 0, nullptr, nullptr) != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    OclLib::finish(ctx->CommandQueues);

    for (int i = 0; i < 4; ++i) {
        if (BranchNonces[i]) {
            // Threads
            if ((OclLib::setKernelArg(ctx->Kernels[i + 3], 4, sizeof(cl_ulong), BranchNonces + i)) != CL_SUCCESS) {
                LOG_ERR(kSetKernelArgErr, err_to_str(ret), i + 3, 4);
                return OCL_ERR_API;
            }

            // round up to next multiple of w_size
            BranchNonces[i] = ((BranchNonces[i] + w_size - 1u) / w_size) * w_size;
            // number of global threads must be a multiple of the work group size (w_size)
            assert(BranchNonces[i]%w_size == 0);
            size_t tmpNonce = ctx->Nonce;
            if ((ret = OclLib::enqueueNDRangeKernel(ctx->CommandQueues, ctx->Kernels[i + 3], 1, &tmpNonce, BranchNonces + i, &w_size, 0, nullptr, nullptr)) != CL_SUCCESS) {
                LOG_ERR("Error %s when calling clEnqueueNDRangeKernel for kernel %d.", err_to_str(ret), i + 3);
                return OCL_ERR_API;
            }
        }
    }

    if (OclLib::enqueueReadBuffer(ctx->CommandQueues, ctx->OutputBuffer, CL_TRUE, 0, sizeof(cl_uint) * 0x100, HashOutput, 0, nullptr, NULL) != CL_SUCCESS) {
        return OCL_ERR_API;
    }

    OclLib::finish(ctx->CommandQueues);
    auto & numHashValues = HashOutput[0xFF];
    // avoid out of memory read, we have only storage for 0xFF results
    if (numHashValues > 0xFF) {
        numHashValues = 0xFF;
    }

    ctx->Nonce += (uint32_t) g_intensity;

    return OCL_ERR_SUCCESS;
}


void ReleaseOpenCl(GpuContext* ctx)
{
    OclLib::releaseMemObject(ctx->InputBuffer);
    OclLib::releaseMemObject(ctx->OutputBuffer);

    int buffer_count = sizeof(ctx->ExtraBuffers) / sizeof(ctx->ExtraBuffers[0]);
    for (int b = 0; b < buffer_count; ++b) {
        OclLib::releaseMemObject(ctx->ExtraBuffers[b]);
    }

    OclLib::releaseProgram(ctx->Program);

    int kernel_count = sizeof(ctx->Kernels) / sizeof(ctx->Kernels[0]);
    for (int k = 0; k < kernel_count; ++k) {
        OclLib::releaseKernel(ctx->Kernels[k]);
    }

    OclLib::releaseCommandQueue(ctx->CommandQueues);
}


void ReleaseOpenClContext(cl_context opencl_ctx)
{
    OclLib::releaseContext(opencl_ctx);
}
