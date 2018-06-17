/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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
#include "base32/base32.h"
#include "common/crypto/keccak.h"
#include "common/log/Log.h"
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


bool OclCache::load()
{
    const xmrig::Algo algo = m_config->algorithm().algo();

    char options[512] = { 0 };
    snprintf(options, sizeof(options), "-DITERATIONS=%u -DMASK=%u -DWORKSIZE=%zu -DSTRIDED_INDEX=%d -DMEM_CHUNK_EXPONENT=%d -DCOMP_MODE=%d -DMEMORY=%zu -DALGO=%d",
             xmrig::cn_select_iter(algo, xmrig::VARIANT_0),
             xmrig::cn_select_mask(algo),
             m_ctx->workSize,
             m_ctx->stridedIndex,
             static_cast<int>(1u << m_ctx->memChunk),
             m_ctx->compMode,
             xmrig::cn_select_memory(algo),
             static_cast<int>(algo)
             );

    if (!prepare(options)) {
        return false;
    }

    std::ifstream clBinFile(m_fileName, std::ofstream::in | std::ofstream::binary);
    cl_int ret;

    if (!m_config->isOclCache() || !clBinFile.good()) {
        LOG_INFO(m_config->isColors() ? "GPU " WHITE_BOLD("#%zu") " " YELLOW("compiling...") :
                                        "GPU #%zu compiling...", m_ctx->deviceIdx);

        m_ctx->Program = clCreateProgramWithSource(m_oclCtx, 1, reinterpret_cast<const char**>(&m_sourceCode), nullptr, &ret);
        if (ret != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clCreateProgramWithSource on the OpenCL miner code", OclError::toString(ret));
            return false;
        }

        ret = clBuildProgram(m_ctx->Program, 1, &m_ctx->DeviceID, options, nullptr, nullptr);
        if (ret != CL_SUCCESS) {
            size_t len = 0;
            LOG_ERR("Error %s when calling clBuildProgram.", OclError::toString(ret));

            if ((ret = clGetProgramBuildInfo(m_ctx->Program, m_ctx->DeviceID, CL_PROGRAM_BUILD_LOG, 0, nullptr, &len)) != CL_SUCCESS) {
                LOG_ERR("Error %s when calling clGetProgramBuildInfo for length of build log output.", OclError::toString(ret));
                return false;
            }

            char *buildLog = new char[len + 1]();

            if ((ret = clGetProgramBuildInfo(m_ctx->Program, m_ctx->DeviceID, CL_PROGRAM_BUILD_LOG, len, buildLog, nullptr)) != CL_SUCCESS) {
                delete [] buildLog;
                LOG_ERR("Error %s when calling clGetProgramBuildInfo for build log.", OclError::toString(ret));
                return false;
            }

            Log::i()->text("Build log:");
            std::cerr << buildLog << std::endl;

            delete [] buildLog;
            return OCL_ERR_API;
        }

        const cl_uint num_devices = numDevices();
        const int dev_id          = devId(num_devices);

        cl_build_status status;
        do
        {
            if ((ret = clGetProgramBuildInfo(m_ctx->Program, m_ctx->DeviceID, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, nullptr)) != CL_SUCCESS) {
                LOG_ERR("Error %s when calling clGetProgramBuildInfo for status of build.", OclError::toString(ret));
                return OCL_ERR_API;
            }

            sleep(1);
        }
        while(status == CL_BUILD_IN_PROGRESS);

        return save(dev_id, num_devices);
    }
    else {
        std::ostringstream ss;
        ss << clBinFile.rdbuf();
        std::string s = ss.str();

        size_t bin_size = s.size();
        auto data_ptr = s.data();

        cl_int clStatus;
        m_ctx->Program = clCreateProgramWithBinary(m_oclCtx, 1, &m_ctx->DeviceID, &bin_size, reinterpret_cast<const unsigned char **>(&data_ptr), &clStatus, &ret);
        if (ret != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clCreateProgramWithBinary. Try to delete file %s", OclError::toString(ret), m_fileName.c_str());
            return false;
        }

        ret = clBuildProgram(m_ctx->Program, 1, &m_ctx->DeviceID, nullptr, nullptr, nullptr);
        if (ret != CL_SUCCESS) {
            LOG_ERR("Error %s when calling clBuildProgram. Try to delete file %s", OclError::toString(ret), m_fileName.c_str());
            return false;
        }
    }

    return true;
}


bool OclCache::prepare(const char *options)
{
    uint8_t state[200] = { 0 };
    char hash[65]      = { 0 };

    if (clGetDeviceInfo(m_ctx->DeviceID, CL_DEVICE_NAME, sizeof state, state, nullptr) != CL_SUCCESS) {
        return false;
    }

    std::string key(m_sourceCode);
    key += options;
    key += reinterpret_cast<const char *>(state);

    xmrig::keccak(key.c_str(), key.size(), state);
    base32_encode(state, 32, reinterpret_cast<uint8_t *>(hash), sizeof hash);

#   ifdef _WIN32
    m_fileName = prefix() + "\\xmrig\\.cache\\" + hash + ".bin";
#   else
    m_fileName = prefix() + "/.cache/" + hash + ".bin";
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
    clGetProgramInfo(m_ctx->Program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * binary_sizes.size(), binary_sizes.data(), nullptr);

    std::vector<char*> all_programs(num_devices);
    std::vector<std::vector<char>> program_storage;

    size_t mem_size = 0;
    for (size_t i = 0; i < all_programs.size(); ++i) {
        program_storage.emplace_back(std::vector<char>(binary_sizes[i]));
        all_programs[i] = program_storage[i].data();
        mem_size += binary_sizes[i];
    }

    cl_int ret;
    if ((ret = clGetProgramInfo(m_ctx->Program, CL_PROGRAM_BINARIES, num_devices * sizeof(char*), all_programs.data(), nullptr)) != CL_SUCCESS) {
        LOG_ERR("Error %s when calling clGetProgramInfo.", OclError::toString(ret));

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
    clGetProgramInfo(m_ctx->Program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, nullptr);

    return num_devices;
}


int OclCache::devId(cl_uint num_devices) const
{
    std::vector<cl_device_id> devices_ids(num_devices);
    clGetProgramInfo(m_ctx->Program, CL_PROGRAM_DEVICES, sizeof(cl_device_id)* devices_ids.size(), devices_ids.data(), nullptr);

    int dev_id = 0;
    for (auto & ocl_device : devices_ids) {
        if (ocl_device == m_ctx->DeviceID) {
            break;
        }

        dev_id++;
    }

    return dev_id;
}
