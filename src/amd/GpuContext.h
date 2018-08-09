/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
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

#ifndef __GPUCONTEXT_H__
#define __GPUCONTEXT_H__


#include "amd/OclLib.h"


#include <stdint.h>
#include <string>


struct GpuContext
{
    inline GpuContext() :
        deviceIdx(0),
        rawIntensity(0),
        workSize(0),
        stridedIndex(1),
        memChunk(2),
        compMode(1),
        DeviceID(nullptr),
        CommandQueues(nullptr),
        InputBuffer(nullptr),
        OutputBuffer(nullptr),
        ExtraBuffers{ nullptr },
        Program(nullptr),
        Kernels{ nullptr },
        freeMem(0),
        computeUnits(0),
        Nonce(0)
    {}


    inline GpuContext(size_t index, size_t intensity, size_t worksize, int stridedIndex, int memChunk, bool compMode) :
        deviceIdx(index),
        rawIntensity(intensity),
        workSize(worksize),
        stridedIndex(stridedIndex),
        memChunk(memChunk),
        compMode(compMode ? 1 : 0),
        DeviceID(nullptr),
        CommandQueues(nullptr),
        InputBuffer(nullptr),
        OutputBuffer(nullptr),
        ExtraBuffers{ nullptr },
        Program(nullptr),
        Kernels{ nullptr },
        freeMem(0),
        computeUnits(0),
        Nonce(0)
    {}

    void release() { // stops all opencl kernels and releases all opencl resources
        if (CommandQueues) {
            OclLib::finish(CommandQueues);
            OclLib::releaseCommandQueue(CommandQueues);
        }
        if (InputBuffer) OclLib::releaseMemObject(InputBuffer);
        if (OutputBuffer) OclLib::releaseMemObject(OutputBuffer);
        for (int i = 0; i < 6; ++ i)  if (ExtraBuffers[i]) OclLib::releaseMemObject(ExtraBuffers[i]);
        for (int i = 0; i < 11; ++ i) if (Kernels[i]) OclLib::releaseKernel(Kernels[i]);
        if (Program) OclLib::releaseProgram(Program);
    }

    /*Input vars*/
    size_t deviceIdx;
    size_t rawIntensity;
    size_t workSize;
    int stridedIndex;
    int memChunk;
    int compMode;

    /*Output vars*/
    cl_device_id DeviceID;
    cl_command_queue CommandQueues;
    cl_mem InputBuffer;
    cl_mem OutputBuffer;
    cl_mem ExtraBuffers[6];
    cl_program Program;
    cl_kernel Kernels[11];
    size_t freeMem;
    int computeUnits;
    std::string name;

    uint32_t Nonce;
};


#endif /* __GPUCONTEXT_H__ */
