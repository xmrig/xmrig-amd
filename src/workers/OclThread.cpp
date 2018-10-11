/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      SChernykh   <https://github.com/SChernykh>
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


#include <inttypes.h>
#include <stdio.h>
#include <string.h>


#include "rapidjson/document.h"
#include "workers/OclThread.h"
#include "common/log/Log.h"


OclThread::OclThread() :
    m_compMode(true),
    m_memChunk(2),
    m_stridedIndex(2),
    m_unrollFactor(8),
    m_affinity(-1),
    m_index(0),
    m_intensity(0),
    m_worksize(0)
{
}


OclThread::OclThread(const rapidjson::Value &object) :
    m_compMode(true),
    m_memChunk(2),
    m_stridedIndex(2),
    m_unrollFactor(8),
    m_affinity(-1)
{
    setIndex(object["index"].GetInt());
    setIntensity(object["intensity"].GetUint());
    setWorksize(object["worksize"].GetUint());

    const rapidjson::Value &affinity = object["affine_to_cpu"];
    if (affinity.IsInt64()) {
        setAffinity(affinity.GetInt64());
    }

    const rapidjson::Value &stridedIndex = object["strided_index"];
    if (stridedIndex.IsBool()) {
        setStridedIndex(stridedIndex.IsTrue() ? 1 : 0);
    }
    else if (stridedIndex.IsUint()) {
        setStridedIndex(stridedIndex.GetInt());
    }

    // DEPRECATED
    const rapidjson::Value &unrollFactor = object["unroll_factor"];
    if (unrollFactor.IsUint()) {
        setUnrollFactor(unrollFactor.GetInt());
    }

    const rapidjson::Value &unroll = object["unroll"];
    if (unroll.IsUint()) {
        setUnrollFactor(unroll.GetInt());
    }

    const rapidjson::Value &memChunk = object["mem_chunk"];
    if (memChunk.IsUint()) {
        setMemChunk(memChunk.GetInt());
    }

    const rapidjson::Value &compMode = object["comp_mode"];
    if (compMode.IsBool()) {
        setCompMode(compMode.IsTrue());
    }
}


OclThread::OclThread(size_t index, size_t intensity, size_t worksize, int64_t affinity) :
    m_compMode(true),
    m_memChunk(2),
    m_stridedIndex(2),
    m_unrollFactor(8),
    m_affinity(affinity),
    m_index(index),
    m_intensity(intensity),
    m_worksize(worksize)
{
}


OclThread::~OclThread()
{
}


void OclThread::setMemChunk(int memChunk)
{
    if (memChunk >= 0 && memChunk <= 18) {
        m_memChunk = memChunk;
    }
}


void OclThread::setStridedIndex(int stridedIndex)
{
    if (stridedIndex >= 0 && stridedIndex <= 2) {
        m_stridedIndex = stridedIndex;
    }
}


void OclThread::setUnrollFactor(int unrollFactor)
{
    if (unrollFactor < 1) {
        m_unrollFactor = 1;
        return;
    }

    m_unrollFactor = unrollFactor > 128 ? 128 : unrollFactor;
}


#ifdef APP_DEBUG
void OclThread::print() const
{
    LOG_DEBUG(GREEN_BOLD("OpenCL thread:") " index " WHITE_BOLD("%zu") ", intensity " WHITE_BOLD("%zu") ", worksize " WHITE_BOLD("%zu") ",", index(), intensity(), worksize());
    LOG_DEBUG("               strided_index %d, mem_chunk %d, unroll_factor %d, comp_mode %d,", stridedIndex(), memChunk(), unrollFactor(), isCompMode());
    LOG_DEBUG("               affine_to_cpu: %" PRId64, affinity());
}
#endif


#ifndef XMRIG_NO_API
rapidjson::Value OclThread::toAPI(rapidjson::Document &doc) const
{
    return toConfig(doc);
}
#endif


rapidjson::Value OclThread::toConfig(rapidjson::Document &doc) const
{
    using namespace rapidjson;

    Value obj(kObjectType);
    auto &allocator = doc.GetAllocator();

    obj.AddMember("index",         static_cast<uint64_t>(index()),     allocator);
    obj.AddMember("intensity",     static_cast<uint64_t>(intensity()), allocator);
    obj.AddMember("worksize",      static_cast<uint64_t>(worksize()),  allocator);
    obj.AddMember("strided_index", stridedIndex(),                     allocator);
    obj.AddMember("mem_chunk",     memChunk(),                         allocator);
    obj.AddMember("unroll",        unrollFactor(),                     allocator);
    obj.AddMember("comp_mode",     isCompMode(),                       allocator);

    if (affinity() >= 0) {
        obj.AddMember("affine_to_cpu", affinity(), allocator);
    }
    else {
        obj.AddMember("affine_to_cpu", false, allocator);
    }

    return obj;
}
