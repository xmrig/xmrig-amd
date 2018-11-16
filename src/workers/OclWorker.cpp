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


#include <thread>
#include <mutex>

#include "amd/OclGPU.h"
#include "common/Platform.h"
#include "crypto/CryptoNight.h"
#include "workers/Handle.h"
#include "workers/OclThread.h"
#include "workers/OclWorker.h"
#include "workers/Workers.h"
#include "core/Config.h"
#include "common/log/Log.h"

#define MAX_DEVICE_COUNT 32

static struct SGPUThreadInterleaveData
{
    std::mutex m;

    double interleaveAdjustThreshold;
    double averageRunTime;
    uint64_t lastRunTimeStamp;
    int threadCount;
} GPUThreadInterleaveData[MAX_DEVICE_COUNT];

OclWorker::OclWorker(Handle *handle, xmrig::Config *config) :
    m_id(handle->threadId()),
    m_threads(handle->totalWays()),
    m_ctx(handle->ctx()),
    m_hashCount(0),
    m_timestamp(0),
    m_count(0),
    m_sequence(0),
    m_blob(),
    m_config(config)
{
    const int64_t affinity = handle->config()->affinity();

    if (affinity >= 0) {
        Platform::setThreadAffinity(affinity);
    }

    SGPUThreadInterleaveData& interleaveData = GPUThreadInterleaveData[m_ctx->deviceIdx % MAX_DEVICE_COUNT];
    {
        std::lock_guard<std::mutex> g(interleaveData.m);
        ++interleaveData.threadCount;
        interleaveData.interleaveAdjustThreshold = 0.95;
    }
}

void OclWorker::start()
{
    SGPUThreadInterleaveData& interleaveData = GPUThreadInterleaveData[m_ctx->deviceIdx % MAX_DEVICE_COUNT];
    cl_uint results[0x100];

    while (Workers::sequence() > 0) {
        while (!Workers::isOutdated(m_sequence)) {
            memset(results, 0, sizeof(cl_uint) * (0x100));

            using namespace std::chrono;

            const int64_t t0 = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();
            int64_t interleaveAdjustDelay = 0;
            {
                std::lock_guard<std::mutex> g(interleaveData.m);

                const int64_t dt = static_cast<int64_t>(t0 - interleaveData.lastRunTimeStamp);
                interleaveData.lastRunTimeStamp = t0;

                // The perfect interleaving is when N threads on the same GPU start with T/N interval between each other
                // If a thread starts earlier than 0.75*T/N ms after the previous thread, delay it to restore perfect interleaving
                if ((interleaveData.threadCount > 1) && (dt > 0) && (dt < interleaveData.interleaveAdjustThreshold * (interleaveData.averageRunTime / interleaveData.threadCount))) {
                    interleaveAdjustDelay = static_cast<int64_t>(interleaveData.averageRunTime / interleaveData.threadCount - dt);
                    interleaveData.interleaveAdjustThreshold = 0.75;
                }
            }

            if (interleaveAdjustDelay > 0) {
                if (interleaveAdjustDelay >= 400) {
                    interleaveAdjustDelay = 200;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(interleaveAdjustDelay));
                LOG_INFO(m_config->isColors() ?
                    "Thread " WHITE_BOLD("#%zu") " was paused for " YELLOW_BOLD("%lld") " ms to adjust interleaving" :
                    "Thread #%zu was paused for %lld ms to adjust interleaving", m_id, interleaveAdjustDelay);
            }

            const int64_t t1 = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();

            XMRRunJob(m_ctx, results, m_job.algorithm().variant());

            const int64_t t2 = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();

            for (size_t i = 0; i < results[0xFF]; i++) {
                *m_job.nonce() = results[i];
                Workers::submit(m_job);
            }

            m_count += m_ctx->rawIntensity;

            if (t2 > t1) {
                // averagingBias = 1.0 - only the last delta time is taken into account
                // averagingBias = 0.5 - the last delta time has the same weight as all the previous ones combined
                // averagingBias = 0.1 - the last delta time has 10% weight of all the previous ones combined
                const double averagingBias = 0.1;

                std::lock_guard<std::mutex> g(interleaveData.m);
                interleaveData.averageRunTime = interleaveData.averageRunTime * (1.0 - averagingBias) + (t2 - t1) * averagingBias;
            }

            storeStats();
            std::this_thread::yield();
        }

        if (Workers::isPaused()) {
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } while (Workers::isPaused());

            if (Workers::sequence() == 0) {
                break;
            }
        }

        consumeJob();
    }
}


bool OclWorker::resume(const Job &job)
{
    if (m_job.poolId() == -1 && job.poolId() >= 0 && job.id() == m_pausedJob.id()) {
        m_job   = m_pausedJob;
        m_nonce = m_pausedNonce;

        m_ctx->Nonce = m_nonce;

        return true;
    }

    return false;
}


void OclWorker::consumeJob()
{
    Job job = Workers::job();
    m_sequence = Workers::sequence();
    if (m_job == job) {
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
        m_nonce = (*m_job.nonce() & 0xff000000U) + (0xffffffU / m_threads * m_id);
    }
    else {
        m_nonce = 0xffffffffU / m_threads * m_id;
    }

    m_ctx->Nonce = m_nonce;

    setJob();
}


void OclWorker::save(const Job &job)
{
    if (job.poolId() == -1 && m_job.poolId() >= 0) {
        m_pausedJob   = m_job;
        m_pausedNonce = m_nonce;
    }
}


void OclWorker::setJob()
{
    memcpy(m_blob, m_job.blob(), sizeof(m_blob));

    XMRSetJob(m_ctx, m_blob, m_job.size(), m_job.target(), m_job.algorithm().variant());
}


void OclWorker::storeStats()
{
    using namespace std::chrono;

    const uint64_t timestamp = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();
    m_hashCount.store(m_count, std::memory_order_relaxed);
    m_timestamp.store(timestamp, std::memory_order_relaxed);
}
