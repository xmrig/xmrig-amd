/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
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

#ifndef __OCLTHREAD_H__
#define __OCLTHREAD_H__


#include <vector>


#include "amd/GpuContext.h"


class OclThread
{
public:
    OclThread();
    OclThread(size_t index, size_t intensity, size_t worksize, int affinity = -1);
    ~OclThread();

    inline int affinity() const     { return m_affinity; }
    inline int threadId() const     { return m_threadId; }
    inline size_t index() const     { return m_index; }
    inline size_t intensity() const { return m_intensity; }
    inline size_t worksize() const  { return m_worksize; }

    inline void setAffinity(int affinity)      { m_affinity = affinity; }
    inline void setIndex(size_t index)         { m_index = index; }
    inline void setIntensity(size_t intensity) { m_intensity = intensity; }
    inline void setThreadId(int threadId)      { m_threadId = threadId; }
    inline void setWorksize(size_t worksize)   { m_worksize = worksize; }

private:
    int m_affinity;
    int m_threadId;
    size_t m_index;
    size_t m_intensity;
    size_t m_worksize;
};


#endif /* __OCLTHREAD_H__ */
