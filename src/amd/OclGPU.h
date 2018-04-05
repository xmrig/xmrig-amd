/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
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


#ifndef __OCLGPU_H__
#define __OCLGPU_H__


#include <vector>


#include "amd/GpuContext.h"
#include "xmrig.h"


#define OCL_ERR_SUCCESS    (0)
#define OCL_ERR_API        (2)
#define OCL_ERR_BAD_PARAMS (1)


uint32_t getNumPlatforms();
void printPlatforms();
int getAMDPlatformIdx();
std::vector<GpuContext> getAMDDevices(int index);

size_t InitOpenCL(GpuContext* ctx, size_t num_gpus, size_t platform_idx);
size_t XMRSetJob(GpuContext* ctx, uint8_t* input, size_t input_len, uint64_t target, xmrig::Algo algorithm, uint32_t variant);
size_t XMRRunJob(GpuContext* ctx, cl_uint* HashOutput, xmrig::Algo algorithm, uint32_t variant);

#endif /* __OCLGPU_H__ */
