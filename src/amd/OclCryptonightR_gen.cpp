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

#include <cstring>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>


#include "amd/OclCache.h"
#include "amd/OclCryptonightR_gen.h"
#include "amd/OclError.h"
#include "amd/OclLib.h"
#include "common/log/Log.h"
#include "crypto/CryptoNight_monero.h"


static std::string get_code(const V4_Instruction* code, int code_size)
{
    std::stringstream s;

    for (int i = 0; i < code_size; ++i)
    {
        const V4_Instruction inst = code[i];

        const uint32_t a = inst.dst_index;
        const uint32_t b = inst.src_index;

        switch (inst.opcode)
        {
        case MUL:
            s << 'r' << a << "*=r" << b << ';';
            break;

        case ADD:
            s << 'r' << a << "+=r" << b << '+' << inst.C << "U;";
            break;

        case SUB:
            s << 'r' << a << "-=r" << b << ';';
            break;

        case ROR:
        case ROL:
            s << 'r' << a << "=rotate(r" << a << ((inst.opcode == ROR) ? ",ROT_BITS-r" : ",r") << b << ");";
            break;

        case XOR:
            s << 'r' << a << "^=r" << b << ';';
            break;
        }

        s << '\n';
    }

    return s.str();
}

struct CacheEntry
{
    CacheEntry(xmrig::Variant variant, uint64_t height, size_t deviceIdx, std::string&& hash, cl_program program) :
        variant(variant),
        height(height),
        deviceIdx(deviceIdx),
        hash(std::move(hash)),
        program(program)
    {}

    xmrig::Variant variant;
    uint64_t height;
    size_t deviceIdx;
    std::string hash;
    cl_program program;
};

struct BackgroundTaskBase
{
    virtual ~BackgroundTaskBase() {}
    virtual void exec() = 0;
};

template<typename T>
struct BackgroundTask : public BackgroundTaskBase
{
    BackgroundTask(T&& func) : m_func(std::move(func)) {}
    void exec() override { m_func(); }

    T m_func;
};

static std::mutex CryptonightR_cache_mutex;
static std::mutex CryptonightR_build_mutex;
static std::vector<CacheEntry> CryptonightR_cache;

static std::mutex background_tasks_mutex;
static std::vector<BackgroundTaskBase*> background_tasks;
static std::thread* background_thread = nullptr;

static void background_thread_proc()
{
    std::vector<BackgroundTaskBase*> tasks;
    for (;;) {
        tasks.clear();
        {
            std::lock_guard<std::mutex> g(background_tasks_mutex);
            background_tasks.swap(tasks);
        }

        for (BackgroundTaskBase* task : tasks) {
            task->exec();
            delete task;
        }

        OclCache::sleep(500);
    }
}

template<typename T>
static void background_exec(T&& func)
{
    BackgroundTaskBase* task = new BackgroundTask<T>(std::move(func));

    std::lock_guard<std::mutex> g(background_tasks_mutex);
    background_tasks.push_back(task);
    if (!background_thread) {
        background_thread = new std::thread(background_thread_proc);
    }
}


static cl_program CryptonightR_build_program(const GpuContext *ctx, xmrig::Variant variant, uint64_t height, const std::string &source, const std::string &options, std::string hash)
{
    std::vector<cl_program> old_programs;
    old_programs.reserve(32);
    {
        std::lock_guard<std::mutex> g(CryptonightR_cache_mutex);

        // Remove old programs from cache
        for (size_t i = 0; i < CryptonightR_cache.size();)
        {
            const CacheEntry& entry = CryptonightR_cache[i];
            if ((entry.variant == variant) && (entry.height + PRECOMPILATION_DEPTH < height))
            {
                LOG_DEBUG("CryptonightR: program for height %" PRIu64 " released (old program)", entry.height);
                old_programs.push_back(entry.program);
                CryptonightR_cache[i] = std::move(CryptonightR_cache.back());
                CryptonightR_cache.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }

    for (cl_program p : old_programs) {
        OclLib::releaseProgram(p);
    }

    std::lock_guard<std::mutex> g1(CryptonightR_build_mutex);

    cl_program program = nullptr;
    {
        std::lock_guard<std::mutex> g(CryptonightR_cache_mutex);

        // Check if the cache already has this program (some other thread might have added it first)
        for (const CacheEntry& entry : CryptonightR_cache)
        {
            if ((entry.variant == variant) && (entry.height == height) && (entry.deviceIdx == ctx->deviceIdx) && (entry.hash == hash))
            {
                program = entry.program;
                break;
            }
        }
    }

    if (program) {
        return program;
    }

    cl_int ret;
    const char* s = source.c_str();
    program = OclLib::createProgramWithSource(ctx->opencl_ctx, 1, &s, nullptr, &ret);
    if (ret != CL_SUCCESS)
    {
        LOG_ERR("CryptonightR: clCreateProgramWithSource returned error %s", OclError::toString(ret));
        return nullptr;
    }

    ret = OclLib::buildProgram(program, 1, &ctx->DeviceID, options.c_str());
    if (ret != CL_SUCCESS) {
        LOG_ERR("CryptonightR: clBuildProgram returned error %s", OclError::toString(ret));
        printf("Build log:\n%s\n", OclLib::getProgramBuildLog(program, ctx->DeviceID).data());

        OclLib::releaseProgram(program);
        return nullptr;
    }

    ret = OclCache::wait_build(program, ctx->DeviceID);
    if (ret != CL_SUCCESS) {
        OclLib::releaseProgram(program);
        LOG_ERR("CryptonightR: wait_build returned error %s", OclError::toString(ret));
        return nullptr;
    }

    LOG_DEBUG("CryptonightR: programs for heights %" PRIu64 " - %" PRIu64 " compiled", height * 10, height * 10 + 9);

    {
        std::lock_guard<std::mutex> g(CryptonightR_cache_mutex);
        CryptonightR_cache.emplace_back(variant, height, ctx->deviceIdx, std::move(hash), program);
    }
    return program;
}


static bool is_64bit(xmrig::Variant variant)
{
    return false;
}


cl_program CryptonightR_get_program(GpuContext* ctx, xmrig::Variant variant, uint64_t height, bool background)
{
    if (background) {
        background_exec([=](){ CryptonightR_get_program(ctx, variant, height, false); });
        return nullptr;
    }

    std::string source_code = 
        #include "opencl/wolf-aes.cl"
        #include "opencl/cryptonight_r_defines.cl"
    ;

    const char* source_code_template =
        #include "opencl/cryptonight_r.cl"
    ;

    const char include_name[] = "XMRIG_INCLUDE_RANDOM_MATH";
    const char* offset = strstr(source_code_template, include_name);
    if (!offset)
    {
        LOG_ERR("CryptonightR_get_program: XMRIG_INCLUDE_RANDOM_MATH not found in cryptonight_r.cl", variant);
        return nullptr;
    }

    for (int i = 0; i < 10; ++i)
    {
        V4_Instruction code[256];
        int code_size;
        switch (variant)
        {
        case xmrig::VARIANT_WOW:
            code_size = v4_random_math_init<xmrig::VARIANT_WOW>(code, height * 10 + i);
            break;
        case xmrig::VARIANT_4:
            code_size = v4_random_math_init<xmrig::VARIANT_4>(code, height * 10 + i);
            break;
        default:
            LOG_ERR("CryptonightR_get_program: invalid variant %d", variant);
            return nullptr;
        }

        std::string s(source_code_template, offset);
        s.append(get_code(code, code_size));
        s.append(offset + sizeof(include_name) - 1);

        const char kernel_name[] = "cn1_cryptonight_r_N";
        s[s.find(kernel_name) + sizeof(kernel_name) - 2] = '0' + i;

        source_code += s;
    }

    char options[512] = {};
    OclCache::getOptions(xmrig::CRYPTONIGHT, variant, ctx, options, sizeof(options));

    char variant_buf[64];
    snprintf(variant_buf, sizeof(variant_buf), " -DVARIANT=%d", static_cast<int>(variant));
    strcat(options, variant_buf);

    if (is_64bit(variant))
    {
        strcat(options, " -DRANDOM_MATH_64_BIT");
    }

    const char* source = source_code.c_str();
    std::string hash;
    if (ctx->DeviceString.empty() && !OclCache::get_device_string(ctx->platformIdx, ctx->DeviceID, ctx->DeviceString)) {
        return nullptr;
    }
    OclCache::calc_hash(ctx->DeviceString, source, options, hash);

    {
        std::lock_guard<std::mutex> g(CryptonightR_cache_mutex);

        // Check if the cache has this program
        for (const CacheEntry& entry : CryptonightR_cache)
        {
            if ((entry.variant == variant) && (entry.height == height) && (entry.deviceIdx == ctx->deviceIdx) && (entry.hash == hash))
            {
                LOG_DEBUG("CryptonightR: program for height %" PRIu64 " found in cache", height);
                return entry.program;
            }
        }
    }

    return CryptonightR_build_program(ctx, variant, height, source, options, hash);
}
