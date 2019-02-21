/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
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
#include <mutex>
#include <thread>


#include "amd/OclGPU.h"
#include "common/log/Log.h"
#include "common/Platform.h"
#include "common/utils/timestamp.h"
#include "core/Config.h"
#include "crypto/CryptoNight.h"
#include "workers/Handle.h"
#include "workers/OclThread.h"
#include "workers/OclWorker.h"
#include "workers/Workers.h"


#define MAX_DEVICE_COUNT 32


static struct SGPUThreadInterleaveData
{
    std::mutex m;

    double adjustThreshold   = 0.95;
    double averageRunTime    = 0;
    int64_t lastRunTimeStamp = 0;
    int resumeCounter        = 0;
} GPUThreadInterleaveData[MAX_DEVICE_COUNT];


OclWorker::OclWorker(Handle *handle) :
    m_id(handle->threadId()),
    m_threads(handle->totalWays()),
    m_ctx(handle->ctx()),
    m_hashCount(0),
    m_timestamp(0),
    m_count(0),
    m_sequence(0),
    m_blob()
{
    const int64_t affinity = handle->config()->affinity();

    if (affinity >= 0) {
        Platform::setThreadAffinity(static_cast<uint64_t>(affinity));
    }
}


void OclWorker::start()
{
    SGPUThreadInterleaveData& interleaveData = GPUThreadInterleaveData[m_ctx->deviceIdx % MAX_DEVICE_COUNT];
    cl_uint results[0x100];

    while (Workers::sequence() > 0) {
        while (!Workers::isOutdated(m_sequence)) {
            memset(results, 0, sizeof(cl_uint) * (0x100));

            const int64_t delay = interleaveAdjustDelay();
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));

#               ifdef APP_INTERLEAVE_DEBUG
                LOG_WARN("Thread #%zu was paused for %" PRId64 " ms to adjust interleaving", m_id, delay);
#               endif
            }

            const int64_t t = xmrig::steadyTimestamp();

            XMRRunJob(m_ctx, results, m_job.algorithm().variant());

            for (size_t i = 0; i < results[0xFF]; i++) {
                *m_job.nonce() = results[i];
                Workers::submit(m_job);
            }

            storeStats(t);
            std::this_thread::yield();
        }

        if (Workers::isPaused()) {
            {
                std::lock_guard<std::mutex> g(interleaveData.m);
                interleaveData.resumeCounter = 0;
            }

            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } while (Workers::isPaused());

            if (Workers::sequence() == 0) {
                break;
            }

            const int64_t delay = resumeDelay();
            if (delay > 0) {
#               ifdef APP_INTERLEAVE_DEBUG
                LOG_WARN("Thread #%zu will be paused for %" PRId64 " ms to before resuming", m_id, delay);
#               endif

                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
        }

        consumeJob();
    }
}


bool OclWorker::resume(const xmrig::Job &job)
{
    if (m_job.poolId() == -1 && job.poolId() >= 0 && job.id() == m_pausedJob.id()) {
        m_job        = m_pausedJob;
        m_ctx->Nonce = m_pausedNonce;

        return true;
    }

    return false;
}


int64_t OclWorker::interleaveAdjustDelay() const
{
    SGPUThreadInterleaveData &data = GPUThreadInterleaveData[m_ctx->deviceIdx % MAX_DEVICE_COUNT];

    const int64_t t0 = xmrig::steadyTimestamp();
    int64_t delay    = 0;

    {
        std::lock_guard<std::mutex> g(data.m);

        const int64_t dt = t0 - data.lastRunTimeStamp;
        data.lastRunTimeStamp = t0;

        // The perfect interleaving is when N threads on the same GPU start with T/N interval between each other
        // If a thread starts earlier than 0.75*T/N ms after the previous thread, delay it to restore perfect interleaving
        if ((m_ctx->threads > 1) && (dt > 0) && (dt < data.adjustThreshold * (data.averageRunTime / m_ctx->threads))) {
            delay = static_cast<int64_t>(data.averageRunTime / m_ctx->threads - dt);
            data.adjustThreshold = 0.75;
        }
    }

    if (delay >= 400) {
        delay = 200;
    }

    return delay;
}


int64_t OclWorker::resumeDelay() const
{
    SGPUThreadInterleaveData &data = GPUThreadInterleaveData[m_ctx->deviceIdx % MAX_DEVICE_COUNT];

    int64_t delay = 0;

    {
        constexpr const double firstThreadSpeedupCoeff = 1.25;

        std::lock_guard<std::mutex> g(data.m);
        delay = static_cast<int64_t>(data.resumeCounter * data.averageRunTime / m_ctx->threads / firstThreadSpeedupCoeff);
        ++data.resumeCounter;
    }

    if (delay > 1000) {
        delay = 1000;
    }

    return delay;
}


void OclWorker::consumeJob()
{
    xmrig::Job job = Workers::job();
    m_sequence = Workers::sequence();
    if (m_job.id() == job.id() && m_job.clientId() == job.clientId()) {
        return;
    }

    save(job);

    if (resume(job)) {
        setJob();
        return;
    }

    m_job = std::move(job);
    m_job.setThreadId(m_id);

    if (m_job.isNicehash()) {
        m_ctx->Nonce = (*m_job.nonce() & 0xff000000U) + (0xffffffU / m_threads * m_id);
    }
    else {
        m_ctx->Nonce = 0xffffffffU / m_threads * m_id;
    }

    setJob();
}


void OclWorker::save(const xmrig::Job &job)
{
    if (job.poolId() == -1 && m_job.poolId() >= 0) {
        m_pausedJob   = m_job;
        m_pausedNonce = m_ctx->Nonce;
    }
}


void OclWorker::setJob()
{
    memcpy(m_blob, m_job.blob(), sizeof(m_blob));

    XMRSetJob(m_ctx, m_blob, m_job.size(), m_job.target(), m_job.algorithm().variant(), m_job.height());
}


void OclWorker::storeStats(int64_t t)
{
    if (Workers::isPaused()) {
        return;
    }

    SGPUThreadInterleaveData &data = GPUThreadInterleaveData[m_ctx->deviceIdx % MAX_DEVICE_COUNT];

    m_count += m_ctx->rawIntensity;

    // averagingBias = 1.0 - only the last delta time is taken into account
    // averagingBias = 0.5 - the last delta time has the same weight as all the previous ones combined
    // averagingBias = 0.1 - the last delta time has 10% weight of all the previous ones combined
    const double averagingBias = 0.1;

    {
        int64_t t2 = xmrig::steadyTimestamp();

        std::lock_guard<std::mutex> g(data.m);
        data.averageRunTime = data.averageRunTime * (1.0 - averagingBias) + (t2 - t) * averagingBias;
    }

    const uint64_t timestamp = static_cast<uint64_t>(xmrig::currentMSecsSinceEpoch());
    m_hashCount.store(m_count, std::memory_order_relaxed);
    m_timestamp.store(timestamp, std::memory_order_relaxed);
}
