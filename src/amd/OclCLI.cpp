/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      SChernykh   <https://github.com/SChernykh>
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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>


#include "amd/cryptonight.h"
#include "amd/OclCLI.h"
#include "amd/OclGPU.h"
#include "common/log/Log.h"
#include "core/Config.h"
#include "crypto/CryptoNight_constants.h"
#include "workers/OclThread.h"


OclCLI::OclCLI()
{
}


bool OclCLI::setup(std::vector<xmrig::IThread *> &threads)
{
    if (isEmpty()) {
        return false;
    }

    for (size_t i = 0; i < m_devices.size(); i++) {
        OclThread *thread = new OclThread(m_devices[i], intensity(i), worksize(i), affinity(i));
        thread->setStridedIndex(stridedIndex(i));
        thread->setMemChunk(memChunk(i));
        thread->setUnrollFactor(unrollFactor(i));
        thread->setCompMode(compMode(i) == 0 ? false : true);

        threads.push_back(thread);
    }

    return true;
}


void OclCLI::autoConf(std::vector<xmrig::IThread *> &threads, const xmrig::Algorithm& algorithm, xmrig::Config *config)
{
    std::vector<GpuContext> devices = OclGPU::getDevices(config);
    if (devices.empty()) {
        LOG_ERR("No devices found.");
        return;
    }

    const size_t hashMemSize = xmrig::cn_select_memory(algorithm.algo());

    for (const GpuContext &ctx : devices) {
        // Vega APU slow and can cause BSOD, skip from autoconfig.
        if (ctx.name.compare("gfx902") == 0) {
            continue;
        }

        int hints           = getHints(ctx, config);
        if (algorithm.algo() == xmrig::CRYPTONIGHT && algorithm.variant() == xmrig::VARIANT_2) hints |= CNv2;
        const size_t maxThreads   = getMaxThreads(ctx, algorithm.algo(), hints);
        const size_t maxIntensity = getPossibleIntensity(ctx, maxThreads, hashMemSize);
        const size_t computeUnits = static_cast<size_t>(ctx.computeUnits);

        size_t intensity = 0;
        if (hints & Vega) {
            intensity = maxIntensity / computeUnits * computeUnits;
        }
        else {
            intensity = (maxIntensity / (8 * computeUnits)) * computeUnits * 8;
        }

        assert(intensity > 0);
        if (intensity == 0) {
            continue;
        }

        threads.push_back(createThread(ctx, intensity, hints));

        if (hints & DoubleThreads) {
           threads.push_back(createThread(ctx, intensity, hints));
        }
    }
}


void OclCLI::parseLaunch(const char *arg)
{
    char *value = strdup(arg);
    char *pch   = strtok(value, ",");
    std::vector<char *> tmp;

    while (pch != nullptr) {
        tmp.push_back(pch);
        pch = strtok(nullptr, ",");
    }

    for (char *config : tmp) {
        pch       = strtok(config, "x");
        int count = 0;

        while (pch != nullptr && count < 2) {
            count++;

            const int v = (int) strtoul(pch, nullptr, 10);
            if (count == 1) {
                m_intensity.push_back(v > 0 ? v : 0);
            }
            else if (count == 2) {
                m_worksize.push_back(v > 0 ? v : 8);
            }

            pch = strtok(nullptr, "x");
        }

        if (count == 1) {
            m_worksize.push_back(8);
        }
    }

    free(value);
}


int OclCLI::get(const std::vector<int> &vector, int index, int defaultValue) const
{
    if (vector.empty()) {
        return defaultValue;
    }

    if (static_cast<int>(vector.size()) <= index) {
        return vector.back();
    }

    return vector[index];
}


int OclCLI::getHints(const GpuContext &ctx, xmrig::Config *config) const
{
    int hints = None;
    if (config->isCNv2()) {
        hints |= CNv2;
    }

    if (ctx.vendor == xmrig::OCL_VENDOR_AMD && (ctx.name.compare("gfx901") == 0 ||
                                                ctx.name.compare("gfx904") == 0 ||
                                                ctx.name.compare("gfx900") == 0 ||
                                                ctx.name.compare("gfx903") == 0 ||
                                                ctx.name.compare("gfx905") == 0))
    {
        hints |= Vega;
        hints |= DoubleThreads;
    }

    return hints;
}


OclThread *OclCLI::createThread(const GpuContext &ctx, size_t intensity, int hints) const
{
    const size_t worksize = ((hints & Vega) && (hints & CNv2)) ? 16 : 8;

    int stridedIndex = 1;
    if (ctx.vendor == xmrig::OCL_VENDOR_NVIDIA) {
        stridedIndex = 0;
    }
    else if (hints & CNv2) {
        stridedIndex = 2;
    }

    OclThread *thread = new OclThread(ctx.deviceIdx, intensity, worksize);
    thread->setStridedIndex(stridedIndex);

    return thread;
}


void OclCLI::parse(std::vector<int> &vector, const char *arg) const
{
    char *value = strdup(arg);
    char *pch   = strtok(value, ",");

    while (pch != nullptr) {
        vector.push_back((int) strtoul(pch, nullptr, 10));

        pch = strtok(nullptr, ",");
    }

    free(value);
}


size_t OclCLI::getMaxThreads(const GpuContext &ctx, xmrig::Algo algo, int hints)
{
    const size_t ratio = algo == xmrig::CRYPTONIGHT_LITE ? 2u : 1u;
    if (ctx.vendor == xmrig::OCL_VENDOR_INTEL) {
        return ratio * ctx.computeUnits * 8;
    }

    if (hints & Vega) {
        if ((hints & CNv2) && ctx.computeUnits == 56) {
            return 1792u;
        }

        return ratio * 2024u;
    }

    if (ctx.vendor == xmrig::OCL_VENDOR_NVIDIA && (ctx.name.find("P100") != std::string::npos ||
                                                   ctx.name.find("V100") != std::string::npos))
    {
        return 40000u;
    }

    return ratio * 1000u;
}


size_t OclCLI::getPossibleIntensity(const GpuContext &ctx, size_t maxThreads, size_t hashMemSize)
{
    constexpr const size_t byteToMiB = 1024u * 1024u;

    const size_t minFreeMem   = (maxThreads == 40000u ? 512u : 128u) * byteToMiB;
    const size_t availableMem = ctx.freeMem - minFreeMem;
    const size_t perThread    = hashMemSize + 224u;
    const size_t maxIntensity = availableMem / perThread;

    return std::min(maxThreads, maxIntensity);
}
