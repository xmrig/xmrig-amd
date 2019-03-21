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

#ifndef XMRIG_GPUCONTEXT_H
#define XMRIG_GPUCONTEXT_H


#if defined(__APPLE__)
#   include <OpenCL/cl.h>
#else
#   include "3rdparty/CL/cl.h"
#endif


#include <stdint.h>
#include <string>


#include "base/tools/String.h"
#include "common/xmrig.h"


struct GpuContext
{
    inline GpuContext() :
        deviceIdx(0),
        rawIntensity(0),
        workSize(0),
        threads(0),
        stridedIndex(2),
        memChunk(2),
        compMode(1),
        unrollFactor(8),
        vendor(xmrig::OCL_VENDOR_UNKNOWN),
        threadIdx(0),
        opencl_ctx(nullptr),
        platformIdx(0),
        DeviceID(nullptr),
        amdDriverMajorVersion(0),
        CommandQueues(nullptr),
        InputBuffer(nullptr),
        OutputBuffer(nullptr),
        ExtraBuffers{ nullptr },
        Program(nullptr),
        Kernels{ nullptr },
        ProgramCryptonightR(nullptr),
        HeightCryptonightR(0),
        freeMem(0),
        globalMem(0),
        computeUnits(0),
        Nonce(0)
    {
        memset(Kernels, 0, sizeof(Kernels));
    }

    /*Input vars*/
    size_t deviceIdx;
    size_t rawIntensity;
    size_t workSize;
    size_t threads;
    int stridedIndex;
    int memChunk;
    int compMode;
    int unrollFactor;
    xmrig::OclVendor vendor;

    /*Output vars*/
    size_t threadIdx;
    cl_context opencl_ctx;
    int platformIdx;
    cl_device_id DeviceID;
    std::string DeviceString;
    int amdDriverMajorVersion;
    cl_command_queue CommandQueues;
    cl_mem InputBuffer;
    cl_mem OutputBuffer;
    cl_mem ExtraBuffers[6];
    cl_program Program;
    cl_kernel Kernels[32];
    cl_program ProgramCryptonightR;
    uint64_t HeightCryptonightR;
    size_t freeMem;
    size_t globalMem;
    cl_uint computeUnits;
    xmrig::String board;
    xmrig::String name;

    uint32_t Nonce;
};


#endif /* XMRIG_GPUCONTEXT_H */
