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


#include <inttypes.h>
#include <stdio.h>
#include <string.h>


#include "amd/GpuContext.h"
#include "base/io/Json.h"
#include "common/log/Log.h"
#include "rapidjson/document.h"
#include "workers/OclThread.h"


namespace xmrig {

static const char *kAffineToCpu  = "affine_to_cpu";
static const char *kCompMode     = "comp_mode";
static const char *kIndex        = "index";
static const char *kIntensity    = "intensity";
static const char *kMemChunk     = "mem_chunk";
static const char *kStridedIndex = "strided_index";
static const char *kUnroll       = "unroll";
static const char *kWorksize     = "worksize";

}


xmrig::OclThread::OclThread() :
    m_affinity(-1)
{
    m_ctx = new GpuContext();
}


xmrig::OclThread::OclThread(const rapidjson::Value &object) :
    m_affinity(-1)
{
    m_ctx = new GpuContext();

    setIndex(Json::getUint(object, kIndex));
    setIntensity(Json::getUint(object, kIntensity));
    setWorksize(Json::getUint(object, kWorksize));
    setAffinity(Json::getInt64(object, kAffineToCpu, -1));
    setMemChunk(Json::getInt(object, kMemChunk, m_ctx->memChunk));
    setUnrollFactor(Json::getInt(object, kUnroll, m_ctx->unrollFactor));
    setCompMode(Json::getBool(object, kCompMode, true));

    const rapidjson::Value &stridedIndex = object[kStridedIndex];
    if (stridedIndex.IsBool()) {
        setStridedIndex(stridedIndex.IsTrue() ? 1 : 0);
    }
    else if (stridedIndex.IsUint()) {
        setStridedIndex(stridedIndex.GetInt());
    }
}


xmrig::OclThread::OclThread(size_t index, size_t intensity, size_t worksize, int64_t affinity) :
    m_affinity(affinity)
{
    m_ctx = new GpuContext();

    setIndex(index);
    setIntensity(intensity);
    setWorksize(worksize);
}


xmrig::OclThread::~OclThread()
{
    delete m_ctx;
}


size_t xmrig::OclThread::index() const
{
    return m_ctx->deviceIdx;
}


bool xmrig::OclThread::isCompMode() const
{
    return m_ctx->compMode == 1;
}


int xmrig::OclThread::memChunk() const
{
    return m_ctx->memChunk;
}


int xmrig::OclThread::stridedIndex() const
{
    return m_ctx->stridedIndex;
}


int xmrig::OclThread::unrollFactor() const
{
    return m_ctx->unrollFactor;
}


size_t xmrig::OclThread::intensity() const
{
    return m_ctx->rawIntensity;
}


size_t xmrig::OclThread::worksize() const
{
    return m_ctx->workSize;
}


void xmrig::OclThread::setCompMode(bool enable)
{
    m_ctx->compMode = enable ? 1 : 0;
}


void xmrig::OclThread::setIndex(size_t index)
{
    m_ctx->deviceIdx = index;
}


void xmrig::OclThread::setIntensity(size_t intensity)
{
    m_ctx->rawIntensity = intensity;
}


void xmrig::OclThread::setMemChunk(int memChunk)
{
    if (memChunk >= 0 && memChunk <= 18) {
        m_ctx->memChunk = memChunk;
    }
}


void xmrig::OclThread::setStridedIndex(int stridedIndex)
{
    if (stridedIndex >= 0 && stridedIndex <= 2) {
        m_ctx->stridedIndex = stridedIndex;
    }
}


void xmrig::OclThread::setThreadsCountByGPU(size_t threads)
{
    m_ctx->threads = threads;
}


void xmrig::OclThread::setUnrollFactor(int unrollFactor)
{
    if (unrollFactor < 1) {
        m_ctx->unrollFactor = 1;
        return;
    }

    m_ctx->unrollFactor = unrollFactor > 128 ? 128 : unrollFactor;
}


void xmrig::OclThread::setWorksize(size_t worksize)
{
    m_ctx->workSize = worksize;
}


#ifdef APP_DEBUG
void xmrig::OclThread::print() const
{
    LOG_DEBUG(GREEN_BOLD("OpenCL thread:") " index " WHITE_BOLD("%zu") ", intensity " WHITE_BOLD("%zu") ", worksize " WHITE_BOLD("%zu") ",", index(), intensity(), worksize());
    LOG_DEBUG("               strided_index %d, mem_chunk %d, unroll_factor %d, comp_mode %d,", stridedIndex(), memChunk(), unrollFactor(), isCompMode());
    LOG_DEBUG("               affine_to_cpu: %" PRId64, affinity());
}
#endif


#ifndef XMRIG_NO_API
rapidjson::Value xmrig::OclThread::toAPI(rapidjson::Document &doc) const
{
    return toConfig(doc);
}
#endif


rapidjson::Value xmrig::OclThread::toConfig(rapidjson::Document &doc) const
{
    using namespace rapidjson;

    Value obj(kObjectType);
    auto &allocator = doc.GetAllocator();

    obj.AddMember(StringRef(kIndex),        static_cast<uint64_t>(index()),     allocator);
    obj.AddMember(StringRef(kIntensity),    static_cast<uint64_t>(intensity()), allocator);
    obj.AddMember(StringRef(kWorksize),     static_cast<uint64_t>(worksize()),  allocator);
    obj.AddMember(StringRef(kStridedIndex), stridedIndex(),                     allocator);
    obj.AddMember(StringRef(kMemChunk),     memChunk(),                         allocator);
    obj.AddMember(StringRef(kUnroll),       unrollFactor(),                     allocator);
    obj.AddMember(StringRef(kCompMode),     isCompMode(),                       allocator);

    if (affinity() >= 0) {
        obj.AddMember(StringRef(kAffineToCpu), affinity(), allocator);
    }
    else {
        obj.AddMember(StringRef(kAffineToCpu), false, allocator);
    }

    return obj;
}
