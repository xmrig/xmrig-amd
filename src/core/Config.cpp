/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
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

#include <string.h>
#include <uv.h>
#include <inttypes.h>


#include "amd/OclGPU.h"
#include "amd/OclLib.h"
#include "common/config/ConfigLoader.h"
#include "common/log/Log.h"
#include "core/Config.h"
#include "core/ConfigCreator.h"
#include "crypto/CryptoNight_constants.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "workers/OclThread.h"


#ifdef _MSC_VER
#   define strncasecmp _strnicmp
#   define strcasecmp  _stricmp
#endif


static const char *vendors[] = {
    "AMD",
    "NVIDIA",
    "Intel"
};


xmrig::Config::Config() : xmrig::CommonConfig(),
    m_autoConf(false),
    m_cache(true),
    m_shouldSave(false),
    m_platformIndex(0),
#   if defined(__APPLE__)
    m_loader("/System/Library/Frameworks/OpenCL.framework/OpenCL"),
#   elif defined(_WIN32)
    m_loader("OpenCL.dll"),
#   else
    m_loader("libOpenCL.so"),
#   endif
    m_vendor(xmrig::OCL_VENDOR_AMD)
{    
}


bool xmrig::Config::isCNv2() const
{
    if (algorithm().algo() == CRYPTONIGHT_PICO) {
        return true;
    }

    if (algorithm().algo() != CRYPTONIGHT) {
        return false;
    }

    for (const Pool &pool : m_pools.data()) {
        if (cn_base_variant(pool.algorithm().variant()) == VARIANT_2) {
            return true;
        }
    }

    return false;
}


bool xmrig::Config::oclInit()
{
    LOG_WARN("compiling code and initializing GPUs. This will take a while...");

    if (m_vendor != OCL_VENDOR_MANUAL) {
        m_platformIndex = OclGPU::findPlatformIdx(this);
        if (m_platformIndex == -1) {
            LOG_ERR("%s%s OpenCL platform NOT found.", isColors() ? "\x1B[1;31m" : "", vendorName(m_vendor));
            return false;
        }
    }

    if (m_platformIndex >= static_cast<int>(OclLib::getNumPlatforms())) {
        LOG_ERR("%sSelected OpenCL platform index %d doesn't exist.", isColors() ? "\x1B[1;31m" : "", m_platformIndex);
        return false;
    }

    if (m_threads.empty() && !m_oclCLI.setup(m_threads)) {
        m_autoConf   = true;
        m_shouldSave = true;
        m_oclCLI.autoConf(m_threads, this);
    }

    m_threads = filterThreads();
    return !m_threads.empty();
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
    api.AddMember("port",            apiPort(), allocator);
    api.AddMember("access-token",    apiToken() ? Value(StringRef(apiToken())).Move() : Value(kNullType).Move(), allocator);
    api.AddMember("id",              apiId() ? Value(StringRef(apiId())).Move() : Value(kNullType).Move(), allocator);
    api.AddMember("worker-id",       apiWorkerId() ? Value(StringRef(apiWorkerId())).Move() : Value(kNullType).Move(), allocator);
    api.AddMember("ipv6",            isApiIPv6(), allocator);
    api.AddMember("restricted",      isApiRestricted(), allocator);
    doc.AddMember("api",             api, allocator);
    doc.AddMember("autosave",        isAutoSave(), allocator);

    doc.AddMember("background",      isBackground(), allocator);
    doc.AddMember("cache",           isOclCache(), allocator);
    doc.AddMember("colors",          isColors(), allocator);
    doc.AddMember("donate-level",    donateLevel(), allocator);
    doc.AddMember("log-file",        logFile() ? Value(StringRef(logFile())).Move() : Value(kNullType).Move(), allocator);
    doc.AddMember("opencl-platform", vendor() == OCL_VENDOR_MANUAL ? Value(platformIndex()).Move() : Value(StringRef(vendorName(vendor()))).Move(), allocator);
    doc.AddMember("opencl-loader",   StringRef(loader()), allocator);
    doc.AddMember("pools",           m_pools.toJSON(doc), allocator);
    doc.AddMember("print-time",      printTime(), allocator);
    doc.AddMember("retries",         m_pools.retries(), allocator);
    doc.AddMember("retry-pause",     m_pools.retryPause(), allocator);

    Value threads(kArrayType);
    for (const IThread *thread : m_threads) {
        threads.PushBack(thread->toConfig(doc), allocator);
    }
    doc.AddMember("threads", threads, allocator);

    doc.AddMember("user-agent", userAgent() ? Value(StringRef(userAgent())).Move() : Value(kNullType).Move(), allocator);
    doc.AddMember("syslog",     isSyslog(), allocator);
    doc.AddMember("watch",      m_watch, allocator);
}


xmrig::Config *xmrig::Config::load(Process *process, IConfigListener *listener)
{
    return static_cast<Config*>(ConfigLoader::load(process, new ConfigCreator(), listener));
}


const char *xmrig::Config::vendorName(xmrig::OclVendor vendor)
{
    if (vendor == xmrig::OCL_VENDOR_MANUAL) {
        return "manual";
    }

    return vendors[vendor];
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
    case OclCacheKey: /* cache */
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
    case OclDevicesKey: /* --opencl-devices */
        m_oclCLI.parseDevices(arg);
        break;

    case OclLaunchKey: /* --opencl-launch */
        m_oclCLI.parseLaunch(arg);
        break;

    case OclAffinityKey: /* --opencl-affinity */
        m_oclCLI.parseAffinity(arg);
        break;

    case OclSridedIndexKey: /* --opencl-srided-index */
        m_oclCLI.parseStridedIndex(arg);
        break;

    case OclMemChunkKey: /* --opencl-mem-chunk */
        m_oclCLI.parseMemChunk(arg);
        break;

    case OclUnrollKey: /* --opencl-unroll-factor */
        m_oclCLI.parseUnrollFactor(arg);
        break;

    case OclCompModeKey: /* --opencl-comp-mode */
        m_oclCLI.parseCompMode(arg);
        break;

    case OclCacheKey: /* --no-cache */
        return parseBoolean(key, false);

    case OclPrintKey: /* --print-platforms */
        if (OclLib::init(loader())) {
            printPlatforms();
        }
        return false;

    case OclPlatformKey: /* --opencl-platform */
        setPlatformIndex(arg);
        break;

    case OclLoaderKey: /* --opencl-loader */
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
    case OclPlatformKey: /* --opencl-platform */
        setPlatformIndex(static_cast<int>(arg));
        break;

    default:
        break;
    }

    return true;
}


void xmrig::Config::parseJSON(const rapidjson::Document &doc)
{
    CommonConfig::parseJSON(doc);

    const rapidjson::Value &threads = doc["threads"];

    if (threads.IsArray()) {
        for (const rapidjson::Value &value : threads.GetArray()) {
            if (!value.IsObject()) {
                continue;
            }

            if (value.HasMember("intensity")) {
                parseThread(value);
            }
        }
    }
}


std::vector<xmrig::IThread *> xmrig::Config::filterThreads() const
{
    std::vector<IThread *> threads;
    const size_t platform_idx                   = static_cast<size_t>(platformIndex());
    const std::vector<cl_platform_id> platforms = OclLib::getPlatformIDs();

    if (platforms.empty() || platforms.size() <= platform_idx) {
        return threads;
    }

    cl_int ret;
    cl_uint entries;
    if ((ret = OclLib::getDeviceIDs(platforms[platform_idx], CL_DEVICE_TYPE_GPU, 0, nullptr, &entries)) != CL_SUCCESS) {
        return threads;
    }

    for (IThread *thread : m_threads) {
        if (thread->isValid() && thread->index() < entries) {
            threads.push_back(thread);

            continue;
        }

        if (entries <= thread->index()) {
            LOG_ERR("Selected OpenCL device index %zu doesn't exist.", thread->index());
        }

        delete thread;
    }

    return threads;
}


void xmrig::Config::parseThread(const rapidjson::Value &object)
{
    m_threads.push_back(new OclThread(object));
}


void xmrig::Config::setPlatformIndex(const char *name)
{
    constexpr size_t size = sizeof(vendors) / sizeof((vendors)[0]);

    for (size_t i = 0; i < size; i++) {
        if (strcasecmp(name, vendors[i]) == 0) {
            m_vendor = static_cast<OclVendor>(i);
            return;
        }
    }

    setPlatformIndex(strtol(name, nullptr, 10));
}


void xmrig::Config::setPlatformIndex(int index)
{
    if (index < 0) {
        return;
    }

    m_platformIndex = index;
    m_vendor        = xmrig::OCL_VENDOR_MANUAL;
}
