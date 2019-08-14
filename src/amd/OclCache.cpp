/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2019 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2019 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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


#include <fstream>
#include <iostream>
#include <sstream>


#include "amd/OclCache.h"
#include "amd/OclError.h"
#include "amd/OclLib.h"
#include "base32/base32.h"
#include "common/cpu/Cpu.h"
#include "common/crypto/keccak.h"
#include "common/log/Log.h"
#include "common/utils/timestamp.h"
#include "core/Config.h"
#include "crypto/CryptoNight_constants.h"


OclCache::OclCache(int index, cl_context opencl_ctx, GpuContext *ctx, const char *source_code, xmrig::Config *config) :
    m_oclCtx(opencl_ctx),
    m_sourceCode(source_code),
    m_ctx(ctx),
    m_index(index),
    m_config(config)
{
}


cl_int OclCache::wait_build(cl_program program, cl_device_id device)
{
    cl_build_status status;
    do
    {
        if (OclLib::getProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, nullptr) != CL_SUCCESS) {
            return OCL_ERR_API;
        }

        sleep(1);
    } while (status == CL_BUILD_IN_PROGRESS);

    return CL_SUCCESS;
}


void OclCache::getOptions(xmrig::Algo algo, xmrig::Variant, const GpuContext* ctx, char* options, size_t options_size)
{
    snprintf(options, options_size, "-DITERATIONS=%u -DMASK=%u -DWORKSIZE=%zu -DSTRIDED_INDEX=%d -DMEM_CHUNK_EXPONENT=%d -DCOMP_MODE=%d -DMEMORY=%zu "
        "-DALGO=%d -DUNROLL_FACTOR=%d -DOPENCL_DRIVER_MAJOR=%d -DWORKSIZE_GPU=%zu -cl-fp32-correctly-rounded-divide-sqrt",
        xmrig::cn_select_iter(algo, xmrig::VARIANT_AUTO),
        xmrig::cn_select_mask(algo),
        ctx->workSize,
        ctx->stridedIndex,
        static_cast<int>(1u << ctx->memChunk),
        ctx->compMode,
        xmrig::cn_select_memory(algo),
        static_cast<int>(algo),
        ctx->unrollFactor,
        ctx->amdDriverMajorVersion,
        worksize(ctx, xmrig::VARIANT_GPU)
    );
}

bool OclCache::load()
{
    const xmrig::Algo algo  = m_config->algorithm().algo();
    const xmrig::Variant variant = m_config->algorithm().variant();

    char options[512] = { 0 };
    getOptions(algo, variant, m_ctx, options, sizeof(options));

    if (!prepare(options)) {
        return false;
    }

    std::ifstream clBinFile(m_fileName, std::ofstream::in | std::ofstream::binary);

    if (!m_config->isOclCache() || !clBinFile.good()) {
        LOG_INFO(m_config->isColors() ? "GPU " WHITE_BOLD("#%zu") " " YELLOW_BOLD("compiling...") :
                                        "GPU #%zu compiling...", m_ctx->deviceIdx);

        int64_t timeStart = xmrig::steadyTimestamp();

        cl_int ret;
        m_ctx->Program = OclLib::createProgramWithSource(m_oclCtx, 1, reinterpret_cast<const char**>(&m_sourceCode), nullptr, &ret);
        if (ret != CL_SUCCESS) {
            return false;
        }

        if (OclLib::buildProgram(m_ctx->Program, 1, &m_ctx->DeviceID, options) != CL_SUCCESS) {
            printf("Build log:\n%s\n", OclLib::getProgramBuildLog(m_ctx->Program, m_ctx->DeviceID).data());
            return false;
        }

        const cl_uint num_devices = numDevices();
        const int dev_id          = devId(num_devices);

        if (wait_build(m_ctx->Program, m_ctx->DeviceID) != CL_SUCCESS) {
            return false;
        }

        int64_t timeFinish = xmrig::steadyTimestamp();

        LOG_INFO(m_config->isColors() ? "GPU " WHITE_BOLD("#%zu") " " GREEN_BOLD("compilation completed") ", elapsed time " WHITE_BOLD("%.3fs") :
            "GPU #%zu compilation completed, elapsed time %.3fs", m_ctx->deviceIdx, (timeFinish - timeStart) / 1000.0);

        if (!save(dev_id, num_devices)) {
            return false;
        }
    }
    else {
        std::ostringstream ss;
        ss << clBinFile.rdbuf();
        std::string s = ss.str();

        size_t bin_size = s.size();
        auto data_ptr = s.data();

        cl_int clStatus;
        cl_int ret;
        m_ctx->Program = OclLib::createProgramWithBinary(m_oclCtx, 1, &m_ctx->DeviceID, &bin_size, reinterpret_cast<const unsigned char **>(&data_ptr), &clStatus, &ret);
        if (ret != CL_SUCCESS) {
            LOG_NOTICE("Try to delete file %s", m_fileName.c_str());
            return false;
        }

        if (OclLib::buildProgram(m_ctx->Program, 1, &m_ctx->DeviceID) != CL_SUCCESS) {
            LOG_NOTICE("Try to delete file %s", m_fileName.c_str());
            return false;
        }
    }

    return true;
}

bool OclCache::get_device_string(int platform, cl_device_id device, std::string& result)
{
    result.clear();

    uint8_t buf[256] = {};

    if (OclLib::getDeviceInfo(device, CL_DEVICE_NAME, sizeof(buf), buf) != CL_SUCCESS) {
        return false;
    }

    result = reinterpret_cast<const char *>(buf);

#   ifdef XMRIG_STRICT_OPENCL_CACHE
    std::vector<cl_platform_id> platforms = OclLib::getPlatformIDs();
    if (OclLib::getPlatformInfo(platforms[platform], CL_PLATFORM_VERSION, sizeof(buf), buf, nullptr) == CL_SUCCESS) {
        result += reinterpret_cast<const char *>(buf);
    }

    if (OclLib::getDeviceInfo(device, CL_DRIVER_VERSION, sizeof(buf), buf) == CL_SUCCESS) {
        result += reinterpret_cast<const char *>(buf);
    }
#   endif

    if (!xmrig::Cpu::info()->isX64()) {
        result += "x86";
    }

    return true;
}

void OclCache::calc_hash(const std::string& device_string, const char* source_code, const char *options, std::string& hash)
{
    std::string key(source_code);
    key += options;
    key += device_string;

    uint8_t buf[256] = {};
    xmrig::keccak(key.c_str(), key.size(), buf);

    uint8_t buf2[256] = {};
    base32_encode(buf, 32, buf2, sizeof(buf2));
    hash = reinterpret_cast<char*>(buf2);
}

bool OclCache::prepare(const char *options)
{
    std::string device_string;
    if (!get_device_string(m_config->platformIndex(), m_ctx->DeviceID, device_string)) {
        return false;
    }
    calc_hash(device_string, m_sourceCode, options, m_fileName);

#   ifdef _WIN32
    m_fileName = prefix() + "\\xmrig\\.cache\\" + m_fileName + ".bin";
#   else
    m_fileName = prefix() + "/.cache/" + m_fileName + ".bin";
#   endif

#   ifndef XMRIG_STRICT_OPENCL_CACHE
    LOG_INFO("           CACHE: %s", m_fileName.c_str());
#   endif

    return true;
}


bool OclCache::save(int dev_id, cl_uint num_devices) const
{
    if (!m_config->isOclCache()) {
        return true;
    }

    createDirectory();

    std::vector<size_t> binary_sizes(num_devices);
    OclLib::getProgramInfo(m_ctx->Program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * binary_sizes.size(), binary_sizes.data());

    std::vector<char*> all_programs(num_devices);
    std::vector<std::vector<char>> program_storage;

    size_t mem_size = 0;
    for (size_t i = 0; i < all_programs.size(); ++i) {
        program_storage.emplace_back(std::vector<char>(binary_sizes[i]));
        all_programs[i] = program_storage[i].data();
        mem_size += binary_sizes[i];
    }

    if (OclLib::getProgramInfo(m_ctx->Program, CL_PROGRAM_BINARIES, num_devices * sizeof(char*), all_programs.data()) != CL_SUCCESS) {
        return false;
    }

    std::ofstream file_stream;
    file_stream.open(m_fileName, std::ofstream::out | std::ofstream::binary);
    file_stream.write(all_programs[dev_id], binary_sizes[dev_id]);
    file_stream.close();

    return true;
}


cl_uint OclCache::numDevices() const
{
    cl_uint num_devices = 0;
    OclLib::getProgramInfo(m_ctx->Program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices);

    return num_devices;
}


int OclCache::amdDriverMajorVersion(const GpuContext* ctx)
{
#   ifdef XMRIG_STRICT_OPENCL_CACHE
    char buf[64] = { 0 };
    if (OclLib::getDeviceInfo(ctx->DeviceID, CL_DRIVER_VERSION, sizeof buf, buf) != CL_SUCCESS) {
        return 0;
    }

    const int version = strtol(buf, nullptr, 10);

    return version >= 1400 ? version / 100 : 0;
#   else
    return 0;
#   endif
}


size_t OclCache::worksize(const GpuContext *ctx, xmrig::Variant variant)
{
    if (variant != xmrig::VARIANT_GPU) {
        return ctx->workSize;
    }

    size_t maxWorkSize = 0;

    if (OclLib::getDeviceInfo(ctx->DeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkSize) == CL_SUCCESS) {
        maxWorkSize /= 16;

        return ctx->workSize > maxWorkSize ? maxWorkSize : ctx->workSize;
    }

    return ctx->workSize;
}


int OclCache::devId(cl_uint num_devices) const
{
    std::vector<cl_device_id> devices_ids(num_devices);
    OclLib::getProgramInfo(m_ctx->Program, CL_PROGRAM_DEVICES, sizeof(cl_device_id)* devices_ids.size(), devices_ids.data());

    int dev_id = 0;
    for (auto & ocl_device : devices_ids) {
        if (ocl_device == m_ctx->DeviceID) {
            break;
        }

        dev_id++;
    }

    return dev_id;
}
