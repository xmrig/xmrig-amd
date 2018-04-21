/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
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

#ifndef __HANDLE_H__
#define __HANDLE_H__


#include <stdint.h>
#include <uv.h>


#include "xmrig.h"


class IWorker;
class OclThread;
struct GpuContext;


class Handle
{
public:
    Handle(size_t threadId, OclThread *thread, GpuContext *ctx, size_t threads, xmrig::Algo algorithm);

    void join();
    void start(void (*callback) (void *));

    inline const OclThread *gpuThread() const { return m_gpuThread; }
    inline GpuContext *ctx() const            { return m_ctx; }
    inline int threadId() const               { return m_threadId; }
    inline int threads() const                { return m_threads; }
    inline IWorker *worker() const            { return m_worker; }
    inline void setWorker(IWorker *worker)    { m_worker = worker; }
    inline xmrig::Algo algorithm() const      { return m_algorithm; }

private:
    const OclThread *m_gpuThread;
    const size_t m_threadId;
    const size_t m_threads;
    const xmrig::Algo m_algorithm;
    GpuContext *m_ctx;
    IWorker *m_worker;
    uv_thread_t m_thread;
};


#endif /* __HANDLE_H__ */
