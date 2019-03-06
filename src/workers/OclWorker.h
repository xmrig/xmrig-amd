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

#ifndef XMRIG_OCLWORKER_H
#define XMRIG_OCLWORKER_H


#include <atomic>


#include "amd/GpuContext.h"
#include "common/net/Job.h"
#include "common/xmrig.h"
#include "interfaces/IWorker.h"
#include "net/JobResult.h"


class Handle;


class OclWorker : public IWorker
{
public:
    OclWorker(Handle *handle);

protected:
    inline uint64_t hashCount() const override { return m_hashCount.load(std::memory_order_relaxed); }
    inline uint64_t timestamp() const override { return m_timestamp.load(std::memory_order_relaxed); }
    inline bool selfTest() override            { return true; }
    inline size_t id() const override          { return m_id; }

    void start() override;

private:
    bool resume(const xmrig::Job &job);
    int64_t interleaveAdjustDelay() const;
    int64_t resumeDelay() const;
    void consumeJob();
    void save(const xmrig::Job &job);
    void setJob();
    void storeStats(int64_t t);

    const size_t m_id;
    const size_t m_threads;
    GpuContext *m_ctx;
    std::atomic<uint64_t> m_hashCount;
    std::atomic<uint64_t> m_timestamp;
    uint32_t m_pausedNonce;
    uint64_t m_count;
    uint64_t m_sequence;
    uint8_t m_blob[xmrig::Job::kMaxBlobSize];
    xmrig::Job m_job;
    xmrig::Job m_pausedJob;
};


#endif /* XMRIG_OCLWORKER_H */
