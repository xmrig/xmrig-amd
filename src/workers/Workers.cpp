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

#include <cmath>
#include <thread>


#include "amd/OclGPU.h"
#include "api/Api.h"
#include "common/log/Log.h"
#include "core/Config.h"
#include "core/Controller.h"
#include "crypto/CryptoNight.h"
#include "interfaces/IJobResultListener.h"
#include "interfaces/IThread.h"
#include "rapidjson/document.h"
#include "workers/Handle.h"
#include "workers/Hashrate.h"
#include "workers/OclThread.h"
#include "workers/OclWorker.h"
#include "workers/Workers.h"


bool Workers::m_active = false;
bool Workers::m_enabled = true;
Hashrate *Workers::m_hashrate = nullptr;
IJobResultListener *Workers::m_listener = nullptr;
Job Workers::m_job;
size_t Workers::m_threadsCount = 0;
std::atomic<int> Workers::m_paused;
std::atomic<uint64_t> Workers::m_sequence;
std::list<Job> Workers::m_queue;
std::vector<Handle*> Workers::m_workers;
uint64_t Workers::m_ticks = 0;
uv_async_t Workers::m_async;
uv_mutex_t Workers::m_mutex;
uv_rwlock_t Workers::m_rwlock;
uv_timer_t Workers::m_timer;
xmrig::Controller *Workers::m_controller = nullptr;


static std::vector<GpuContext> contexts;


struct JobBaton
{
    uv_work_t request;
    std::vector<Job> jobs;
    std::vector<JobResult> results;
    int errors = 0;

    JobBaton() {
        request.data = this;
    }
};


Job Workers::job()
{
    uv_rwlock_rdlock(&m_rwlock);
    Job job = m_job;
    uv_rwlock_rdunlock(&m_rwlock);

    return job;
}


size_t Workers::hugePages()
{
    return 0;
}


size_t Workers::threads()
{
    return m_threadsCount;
}


void Workers::printHashrate(bool detail)
{
    assert(m_controller != nullptr);
    if (!m_controller) {
        return;
    }

    if (detail) {
        const bool isColors = m_controller->config()->isColors();
        char num1[8] = { 0 };
        char num2[8] = { 0 };
        char num3[8] = { 0 };

        Log::i()->text("%s| THREAD | GPU | 10s H/s | 60s H/s | 15m H/s |", isColors ? "\x1B[1;37m" : "");

        size_t i = 0;
        for (const xmrig::IThread *thread : m_controller->config()->threads()) {
             Log::i()->text("| %6zu | %3zu | %7s | %7s | %7s |",
                            i, thread->index(),
                            Hashrate::format(m_hashrate->calc(i, Hashrate::ShortInterval), num1, sizeof num1),
                            Hashrate::format(m_hashrate->calc(i, Hashrate::MediumInterval), num2, sizeof num2),
                            Hashrate::format(m_hashrate->calc(i, Hashrate::LargeInterval), num3, sizeof num3)
                            );

             i++;
        }
    }

    m_hashrate->print();
}


void Workers::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    if (!m_active) {
        return;
    }

    m_paused = enabled ? 0 : 1;
    m_sequence++;
}


void Workers::setJob(const Job &job, bool donate)
{
    uv_rwlock_wrlock(&m_rwlock);
    m_job = job;

    if (donate) {
        m_job.setPoolId(-1);
    }
    uv_rwlock_wrunlock(&m_rwlock);

    m_active = true;
    if (!m_enabled) {
        return;
    }

    m_sequence++;
    m_paused = 0;
}


bool Workers::start(xmrig::Controller *controller)
{
    m_controller = controller;
    const std::vector<xmrig::IThread *> &threads = controller->config()->threads();
    size_t ways = 0;

    for (const xmrig::IThread *thread : threads) {
       ways += thread->multiway();
    }

    m_threadsCount = threads.size();
    m_hashrate = new Hashrate(m_threadsCount, controller);

    uv_mutex_init(&m_mutex);
    uv_rwlock_init(&m_rwlock);

    m_sequence = 1;
    m_paused   = 1;

    uv_async_init(uv_default_loop(), &m_async, Workers::onResult);

    contexts.resize(m_threadsCount);

    for (size_t i = 0; i < m_threadsCount; ++i) {
        const OclThread *thread = static_cast<OclThread *>(threads[i]);
        contexts[i] = GpuContext(thread->index(), thread->intensity(), thread->worksize(), thread->stridedIndex(), thread->memChunk(), thread->isCompMode());
    }

    if (InitOpenCL(contexts.data(), m_threadsCount, controller->config()) != 0) {
        return false;
    }

    uv_timer_init(uv_default_loop(), &m_timer);
    uv_timer_start(&m_timer, Workers::onTick, 500, 500);

    uint32_t offset = 0;

    size_t i = 0;
    for (xmrig::IThread *thread : threads) {
        Handle *handle = new Handle(i, thread, &contexts[i], offset, ways);
        offset += thread->multiway();
        i++;

        m_workers.push_back(handle);
        handle->start(Workers::onReady);
    }

    return true;
}

void Workers::soft_stop() // stop current workers leaving uv stuff intact (used in switch_algo)
{
    if (m_hashrate) {
        m_hashrate->stop();
        delete m_hashrate;
    }

    m_sequence = 0;
    m_paused   = 0;

    for (size_t i = 0; i < m_workers.size(); ++i) {
        m_workers[i]->join();
        delete m_workers[i];
    }
    m_workers.clear();

    for (size_t i = 0; i < contexts.size(); ++i) contexts[i].release();
}

// setups workers based on specified algorithm (or its basic perf algo more specifically)
bool Workers::switch_algo(const xmrig::Algorithm& algorithm)
{
    if (m_controller->config()->algorithm().algo() == algorithm.algo()) return true;

    soft_stop();

    m_sequence = 1;
    m_paused   = 1;

    const std::vector<xmrig::IThread *> &threads = m_controller->config()->threads(algorithm.algo());
    m_controller->config()->set_algorithm(algorithm);

    Log::i()->text(m_controller->config()->isColors()
        ? GREEN_BOLD(" >>> ") WHITE_BOLD("ALGO CHANGE: ") CYAN_BOLD("%s")
        : " >>> ALGO CHANGE: %s",
        algorithm.name()
    );

    size_t ways = 0;
    for (const xmrig::IThread *thread : threads) {
       ways += thread->multiway();
    }

    m_threadsCount = threads.size();
    m_hashrate = new Hashrate(m_threadsCount, m_controller);

    contexts.resize(m_threadsCount);

    for (size_t i = 0; i < m_threadsCount; ++i) {
        const OclThread *thread = static_cast<OclThread *>(threads[i]);
        contexts[i] = GpuContext(thread->index(), thread->intensity(), thread->worksize(), thread->stridedIndex(), thread->memChunk(), thread->isCompMode());
    }

    if (InitOpenCL(contexts.data(), m_threadsCount, m_controller->config()) != 0) {
        return false;
    }

    uint32_t offset = 0;

    size_t i = 0;
    for (xmrig::IThread *thread : threads) {
        Handle *handle = new Handle(i, thread, &contexts[i], offset, ways);
        offset += thread->multiway();
        i++;

        m_workers.push_back(handle);
        handle->start(Workers::onReady);
    }

    return true;
}

void Workers::stop()
{
    uv_timer_stop(&m_timer);
    m_hashrate->stop();

    uv_close(reinterpret_cast<uv_handle_t*>(&m_async), nullptr);
    m_paused   = 0;
    m_sequence = 0;

    for (size_t i = 0; i < m_workers.size(); ++i) {
        m_workers[i]->join();
    }
}


void Workers::submit(const Job &result)
{
    uv_mutex_lock(&m_mutex);
    m_queue.push_back(result);
    uv_mutex_unlock(&m_mutex);

    uv_async_send(&m_async);
}


#ifndef XMRIG_NO_API
void Workers::threadsSummary(rapidjson::Document &doc)
{
//    uv_mutex_lock(&m_mutex);
//    const uint64_t pages[2] = { m_status.hugePages, m_status.pages };
//    const uint64_t memory   = m_status.ways * xmrig::cn_select_memory(m_status.algo);
//    uv_mutex_unlock(&m_mutex);

//    auto &allocator = doc.GetAllocator();

//    rapidjson::Value hugepages(rapidjson::kArrayType);
//    hugepages.PushBack(pages[0], allocator);
//    hugepages.PushBack(pages[1], allocator);

//    doc.AddMember("hugepages", hugepages, allocator);
//    doc.AddMember("memory", memory, allocator);
}
#endif


void Workers::onReady(void *arg)
{
    auto handle = static_cast<Handle*>(arg);

    IWorker *worker = new OclWorker(handle);
    handle->setWorker(worker);

    start(worker);
}


void Workers::onResult(uv_async_t *handle)
{
    JobBaton *baton = new JobBaton();

    uv_mutex_lock(&m_mutex);
    while (!m_queue.empty()) {
        baton->jobs.push_back(std::move(m_queue.front()));
        m_queue.pop_front();
    }
    uv_mutex_unlock(&m_mutex);

    uv_queue_work(uv_default_loop(), &baton->request,
        [](uv_work_t* req) {
            JobBaton *baton = static_cast<JobBaton*>(req->data);
            if (baton->jobs.empty()) {
                return;
            }

            xmrig::Algo algo = baton->jobs[0].algorithm().algo();
            cryptonight_ctx *ctx = CryptoNight::createCtx(algo);

            for (const Job &job : baton->jobs) {
                JobResult result(job);

                if (job.algorithm().algo() != algo) {
                    CryptoNight::freeCtx(ctx);
                    ctx = CryptoNight::createCtx(algo = job.algorithm().algo());
                }

                if (job.poolId() == -100 || CryptoNight::hash(job, result, ctx)) {
                    baton->results.push_back(result);
                }
                else {
                    baton->errors++;
                }
            }

            CryptoNight::freeCtx(ctx);
        },
        [](uv_work_t* req, int status) {
            JobBaton *baton = static_cast<JobBaton*>(req->data);

            for (const JobResult &result : baton->results) {
                m_listener->onJobResult(result);
            }

            if (baton->errors > 0 && !baton->jobs.empty()) {
                LOG_ERR("THREAD #%d COMPUTE ERROR(s): %i", baton->jobs[0].threadId(), baton->errors);
            }

            delete baton;
        }
    );
}


void Workers::onTick(uv_timer_t *handle)
{
    for (Handle *handle : m_workers) {
        if (!handle->worker()) {
            return;
        }

        m_hashrate->add(handle->threadId(), handle->worker()->hashCount(), handle->worker()->timestamp());
    }

    if ((m_ticks++ & 0xF) == 0)  {
        m_hashrate->updateHighest();
    }
}


void Workers::start(IWorker *worker)
{
    worker->start();
}
