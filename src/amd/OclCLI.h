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

#ifndef XMRIG_OCLCLI_H
#define XMRIG_OCLCLI_H


#include <vector>


#include "common/xmrig.h"


struct GpuContext;


namespace xmrig {
    class Config;
    class IThread;
    class OclThread;
}


class OclCLI
{
public:
    OclCLI();

    bool setup(std::vector<xmrig::IThread *> &threads);
    void autoConf(std::vector<xmrig::IThread *> &threads, xmrig::Config *config);
    void parseLaunch(const char *arg);

    inline void parseAffinity(const char *arg)     { parse(m_affinity, arg); }
    inline void parseCompMode(const char *arg)     { parse(m_compMode, arg); }
    inline void parseDevices(const char *arg)      { parse(m_devices, arg); }
    inline void parseMemChunk(const char *arg)     { parse(m_memChunk, arg); }
    inline void parseStridedIndex(const char *arg) { parse(m_stridedIndex, arg); }
    inline void parseUnrollFactor(const char *arg) { parse(m_unrollFactor, arg); }

private:
    enum Hints {
        None          = 0,
        DoubleThreads = 1,
        Vega          = 2,
        CNv2          = 4,
        Pico          = 8
    };

    inline bool isEmpty() const                 { return m_devices.empty() && m_intensity.empty(); }
    inline int affinity(size_t index) const     { return get(m_affinity, index, -1); }
    inline int compMode(size_t index) const     { return get(m_compMode, index, 1); }
    inline int intensity(size_t index) const    { return get(m_intensity, index, 0); }
    inline int memChunk(size_t index) const     { return get(m_memChunk, index, 2); }
    inline int stridedIndex(size_t index) const { return get(m_stridedIndex, index, 2); }
    inline int unrollFactor(size_t index) const { return get(m_unrollFactor, index, 8); }
    inline int worksize(size_t index) const     { return get(m_worksize, index, 8); }

    int get(const std::vector<int> &vector, size_t index, int defaultValue) const;
    int getHints(const GpuContext &ctx, xmrig::Config *config) const;
    xmrig::OclThread *createThread(const GpuContext &ctx, size_t intensity, int hints) const;
    void parse(std::vector<int> &vector, const char *arg) const;

    static size_t getMaxThreads(const GpuContext &ctx, xmrig::Algo algo, int hints);
    static size_t getPossibleIntensity(const GpuContext &ctx, size_t maxThreads, size_t hashMemSize);
    static size_t worksizeByHints(int hints);

    std::vector<int> m_affinity;
    std::vector<int> m_compMode;
    std::vector<int> m_devices;
    std::vector<int> m_intensity;
    std::vector<int> m_memChunk;
    std::vector<int> m_stridedIndex;
    std::vector<int> m_unrollFactor;
    std::vector<int> m_worksize;
};


#endif /* XMRIG_OCLCLI_H */
