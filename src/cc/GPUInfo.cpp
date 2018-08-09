/* XMRigCC
 * Copyright 2018-     BenDr0id    <ben@graef.in>
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

#include "GPUInfo.h"

GPUInfo::GPUInfo()
{

}

GPUInfo::~GPUInfo()
{

}

rapidjson::Value GPUInfo::toJson(rapidjson::MemoryPoolAllocator <rapidjson::CrtAllocator>& allocator)
{

}

bool GPUInfo::parseFromJson(const rapidjson::Document& document)
{

}

bool GPUInfo::parseFromJsonString(const std::string& json)
{

}

size_t GPUInfo::getDeviceIdx() const
{
    return m_deviceIdx;
}

void GPUInfo::setDeviceIdx(size_t deviceIdx)
{
    m_deviceIdx = deviceIdx;
}

size_t GPUInfo::getRawIntensity() const
{
    return m_rawIntensity;
}

void GPUInfo::setRawIntensity(size_t rawIntensity)
{
    m_rawIntensity = rawIntensity;
}

size_t GPUInfo::getWorkSize() const
{
    return m_workSize;
}

void GPUInfo::setWorkSize(size_t workSize)
{
    m_workSize = workSize;
}

size_t GPUInfo::getFreeMem() const
{
    return m_freeMem;
}

void GPUInfo::setFreeMem(size_t freeMem)
{
    m_freeMem = freeMem;
}

int GPUInfo::getStridedIndex() const
{
    return m_stridedIndex;
}

void GPUInfo::setStridedIndex(int stridedIndex)
{
    m_stridedIndex = stridedIndex;
}

int GPUInfo::getMemChunk() const
{
    return m_memChunk;
}

void GPUInfo::setMemChunk(int memChunk)
{
    m_memChunk = memChunk;
}

int GPUInfo::getCompMode() const
{
    return m_compMode;
}

void GPUInfo::setCompMode(int compMode)
{
    m_compMode = compMode;
}

int GPUInfo::getComputeUnits() const
{
    return m_computeUnits;
}

void GPUInfo::setComputeUnits(int computeUnits)
{
    m_computeUnits = computeUnits;
}

std::string GPUInfo::getName() const
{
    return m_name;
}

void GPUInfo::setName(const std::string& name)
{
    m_name = name;
}
