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

#include <string.h>
#include <uv.h>
#include <inttypes.h>


#include "amd/OclGPU.h"
#include "amd/OclLib.h"
#include "common/config/ConfigLoader.h"
#include "common/log/Log.h"
#include "core/Config.h"
#include "core/ConfigCreator.h"
#include "Cpu.h"
#include "crypto/CryptoNight_constants.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "workers/OclThread.h"


// for usage in Client::login to get_algo_perf
namespace xmrig {
    Config* pconfig = nullptr;
};


xmrig::Config::Config() : xmrig::CommonConfig(),
    m_autoConf(false),
    m_cache(true),
    m_shouldSave(false),
    m_platformIndex(0),
#   ifdef _WIN32
    m_loader("OpenCL.dll")
#   else
    m_loader("libOpenCL.so")
#   endif
{
    // not defined algo performance is considered to be 0
    for (int a = 0; a != xmrig::PerfAlgo::PA_MAX; ++ a) {
        const xmrig::PerfAlgo pa = static_cast<xmrig::PerfAlgo>(a);
        m_algo_perf[pa] = 0;
    }
}


xmrig::Config::~Config()
{
}


bool xmrig::Config::oclInit()
{
    LOG_WARN("compiling code and initializing GPUs. This will take a while...");

    for (int a = 0; a != xmrig::PerfAlgo::PA_MAX; ++ a) {
        const xmrig::PerfAlgo pa = static_cast<xmrig::PerfAlgo>(a);
        if (m_threads[pa].empty() && !m_oclCLI.setup(m_threads[pa])) {
            m_autoConf   = true;
            m_shouldSave = true;
            m_oclCLI.autoConf(m_threads[pa], &m_platformIndex, xmrig::Algorithm(pa), this);
        }
    }

    return true;
}


bool xmrig::Config::reload(const char *json)
{
    return xmrig::ConfigLoader::reload(this, json);
}


void xmrig::Config::getJSON(rapidjson::Document &doc) const
{
    using namespace rapidjson;

    doc.SetObject();

    auto &allocator = doc.GetAllocator();

    doc.AddMember("algo", StringRef(algorithm().name()), allocator);

    Value api(kObjectType);
    api.AddMember("port",         apiPort(), allocator);
    api.AddMember("access-token", apiToken() ? Value(StringRef(apiToken())).Move() : Value(kNullType).Move(), allocator);
    api.AddMember("worker-id",    apiWorkerId() ? Value(StringRef(apiWorkerId())).Move() : Value(kNullType).Move(), allocator);
    api.AddMember("ipv6",         isApiIPv6(), allocator);
    api.AddMember("restricted",   isApiRestricted(), allocator);
    doc.AddMember("api",          api, allocator);

    doc.AddMember("background",   isBackground(), allocator);
    doc.AddMember("cache",        isOclCache(), allocator);
    doc.AddMember("colors",       isColors(), allocator);
    doc.AddMember("donate-level", donateLevel(), allocator);
    doc.AddMember("log-file",     logFile() ? Value(StringRef(logFile())).Move() : Value(kNullType).Move(), allocator);
    doc.AddMember("opencl-platform", platformIndex(), allocator);
    doc.AddMember("opencl-loader",   StringRef(loader()), allocator);

    Value pools(kArrayType);

    for (const Pool &pool : m_activePools) {
        pools.PushBack(pool.toJSON(doc), allocator);
    }

    doc.AddMember("pools",         pools, allocator);
    doc.AddMember("print-time",    printTime(), allocator);
    doc.AddMember("retries",       retries(), allocator);
    doc.AddMember("retry-pause",   retryPause(), allocator);

    // save extended "threads" based on m_threads
    Value threads(kObjectType);
    for (int a = 0; a != xmrig::PerfAlgo::PA_MAX; ++ a) {
        const xmrig::PerfAlgo pa = static_cast<xmrig::PerfAlgo>(a);
        Value key(xmrig::Algorithm::perfAlgoName(pa), allocator);
        Value threads2(kArrayType);
        for (const IThread *thread : m_threads[pa]) {
            threads2.PushBack(thread->toConfig(doc), allocator);
        }
        threads.AddMember(key, threads2, allocator);
    }
    doc.AddMember("threads", threads, allocator);

    // save "algo-perf" based on m_algo_perf
    Value algo_perf(kObjectType);
    for (int a = 0; a != xmrig::PerfAlgo::PA_MAX; ++ a) {
        const xmrig::PerfAlgo pa = static_cast<xmrig::PerfAlgo>(a);
        Value key(xmrig::Algorithm::perfAlgoName(pa), allocator);
        algo_perf.AddMember(key, Value(m_algo_perf[pa]), allocator);
    }
    doc.AddMember("algo-perf", algo_perf, allocator);

    doc.AddMember("user-agent", userAgent() ? Value(StringRef(userAgent())).Move() : Value(kNullType).Move(), allocator);
    doc.AddMember("syslog",     isSyslog(), allocator);
    doc.AddMember("watch",      m_watch, allocator);
}


xmrig::Config *xmrig::Config::load(int argc, char **argv, IWatcherListener *listener)
{
    return static_cast<Config*>(ConfigLoader::load(argc, argv, new ConfigCreator(), listener));
}


bool xmrig::Config::finalize()
{
    if (m_state != NoneState) {
        return CommonConfig::finalize();
    }

    if (!CommonConfig::finalize()) {
        return false;
    }

    return true;
}


bool xmrig::Config::parseBoolean(int key, bool enable)
{
    if (!CommonConfig::parseBoolean(key, enable)) {
        return false;
    }

    switch (key) {
    case OclCache: /* cache */
        m_cache = enable;
        break;

    default:
        break;
    }

    return true;
}


bool xmrig::Config::parseString(int key, const char *arg)
{
    if (!CommonConfig::parseString(key, arg)) {
        return false;
    }

    switch (key) {
    case OclDevices: /* --opencl-devices */
        m_oclCLI.parseDevices(arg);
        break;

    case OclLaunch: /* --opencl-launch */
        m_oclCLI.parseLaunch(arg);
        break;

    case OclAffinity: /* --opencl-affinity */
        m_oclCLI.parseAffinity(arg);
        break;

    case OclCache: /* --no-cache */
        return parseBoolean(key, false);

    case OclPrint: /* --print-platforms */
        if (OclLib::init(loader())) {
            printPlatforms();
        }
        return false;

    case OclPlatform: /* --opencl-platform */
        return parseUint64(key, strtol(arg, nullptr, 10));

    case OclLoader: /* --opencl-loader */
        m_loader = arg;
        break;

    default:
        break;
    }

    return true;
}


bool xmrig::Config::parseUint64(int key, uint64_t arg)
{
    if (!CommonConfig::parseUint64(key, arg)) {
        return false;
    }

    switch (key) {
    case OclPlatform: /* --opencl-platform */
        m_platformIndex = static_cast<int>(arg);
        break;

    default:
        break;
    }

    return true;
}

// parse specific perf algo (or generic) threads config
void xmrig::Config::parseThreadsJSON(const rapidjson::Value &threads, const xmrig::PerfAlgo pa)
{
    for (const rapidjson::Value &value : threads.GetArray()) {
        if (!value.IsObject()) {
            continue;
        }

        if (value.HasMember("intensity")) {
            parseThread(value, pa);
        }
    }
}

void xmrig::Config::parseJSON(const rapidjson::Document &doc)
{
    const rapidjson::Value &threads = doc["threads"];

    if (threads.IsArray()) {
        // parse generic (old) threads
        parseThreadsJSON(threads, m_algorithm.perf_algo());
    } else if (threads.IsObject()) {
        // parse new specific perf algo threads
        for (int a = 0; a != xmrig::PerfAlgo::PA_MAX; ++ a) {
            const xmrig::PerfAlgo pa = static_cast<xmrig::PerfAlgo>(a);
            const rapidjson::Value &threads2 = threads[xmrig::Algorithm::perfAlgoName(pa)];
            if (threads2.IsArray()) {
                parseThreadsJSON(threads2, pa);
            }
        }
    }

    const rapidjson::Value &algo_perf = doc["algo-perf"];
    if (algo_perf.IsObject()) {
        for (int a = 0; a != xmrig::PerfAlgo::PA_MAX; ++ a) {
            const xmrig::PerfAlgo pa = static_cast<xmrig::PerfAlgo>(a);
            const rapidjson::Value &key = algo_perf[xmrig::Algorithm::perfAlgoName(pa)];
            if (key.IsDouble()) {
                m_algo_perf[pa] = key.GetDouble();
            }
        }
    }
}


void xmrig::Config::parseThread(const rapidjson::Value &object, const xmrig::PerfAlgo pa)
{
    m_threads[pa].push_back(new OclThread(object));
}
