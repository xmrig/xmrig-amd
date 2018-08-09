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

#ifndef __CONFIG_H__
#define __CONFIG_H__


#include <stdint.h>
#include <vector>


#include "amd/OclCLI.h"
#include "common/config/CommonConfig.h"
#include "common/xmrig.h"
#include "rapidjson/fwd.h"


namespace xmrig {


class ConfigLoader;
class IThread;
class IWatcherListener;


class Config : public CommonConfig
{
public:
    Config();
    ~Config();

    bool oclInit();
    bool reload(const char *json);

    void getJSON(rapidjson::Document &doc) const override;

    inline bool isOclCache() const                       { return m_cache; }
    inline bool isShouldSave() const                     { return m_shouldSave; }
    inline const char *loader() const                    { return m_loader.data(); }
    // access to m_threads taking into accoun that it is now separated for each perf algo
    inline const std::vector<IThread *> &threads(const xmrig::Algo algo = INVALID_ALGO) const {
        return m_threads[algo == INVALID_ALGO ? m_algorithm.algo() : algo];
    }
    inline int platformIndex() const                     { return m_platformIndex; }

    // access to perf algo results
    inline float get_algo_perf(const xmrig::PerfAlgo pa) const             { return m_algo_perf[pa]; }
    inline void set_algo_perf(const xmrig::PerfAlgo pa, const float value) { m_algo_perf[pa] = value; }

    static Config *load(int argc, char **argv, IWatcherListener *listener);

protected:
    bool finalize() override;
    bool parseBoolean(int key, bool enable) override;
    bool parseString(int key, const char *arg) override;
    bool parseUint64(int key, uint64_t arg) override;
    void parseJSON(const rapidjson::Document &doc) override;
    // parse specific perf algo (or generic) threads config
    void parseThreadsJSON(const rapidjson::Value &threads, xmrig::Algo);

private:
    void parseThread(const rapidjson::Value &object, const xmrig::Algo);

    bool m_autoConf;
    bool m_cache;
    bool m_shouldSave;
    int m_platformIndex;
    OclCLI m_oclCLI;
    // threads config for each perf algo
    std::vector<IThread *> m_threads[xmrig::Algo::ALGO_MAX];
    // perf algo hashrate results
    float m_algo_perf[xmrig::PerfAlgo::PA_MAX];
    xmrig::c_str m_loader;
};

extern Config* pconfig;

} /* namespace xmrig */

#endif /* __CONFIG_H__ */
