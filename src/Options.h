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

#ifndef __OPTIONS_H__
#define __OPTIONS_H__


#include <stdint.h>
#include <vector>


#include "amd/OclCLI.h"
#include "rapidjson/fwd.h"
#include "xmrig.h"


class OclThread;
class Url;
struct option;


class Options
{
public:
    enum AlgoVariant {
        AV0_AUTO,
        AV1_AESNI,
        AV2_UNUSED,
        AV3_SOFT_AES,
        AV4_UNUSED,
        AV_MAX
    };

    static inline Options* i() { return m_self; }
    static Options *parse(int argc, char **argv);

    inline bool background() const                        { return m_background; }
    inline bool colors() const                            { return m_colors; }
    inline bool dryRun() const                            { return m_dryRun; }
    inline bool isAutoConf() const                        { return m_autoConf; }
    inline bool syslog() const                            { return m_syslog; }
    inline const char *apiToken() const                   { return m_apiToken; }
    inline const char *apiWorkerId() const                { return m_apiWorkerId; }
    inline const char *configName() const                 { return m_configName; }
    inline const char *logFile() const                    { return m_logFile; }
    inline const char *userAgent() const                  { return m_userAgent; }
    inline const std::vector<OclThread*> &threads() const { return m_threads; }
    inline const std::vector<Url*> &pools() const         { return m_pools; }
    inline int apiPort() const                            { return m_apiPort; }
    inline int donateLevel() const                        { return m_donateLevel; }
    inline int platformIndex() const                      { return m_platformIndex; }
    inline int printTime() const                          { return m_printTime; }
    inline int retries() const                            { return m_retries; }
    inline int retryPause() const                         { return m_retryPause; }
    inline void setColors(bool colors)                    { m_colors = colors; }
    inline xmrig::Algo algorithm() const                  { return m_algorithm; }

    inline static void release()                          { delete m_self; }

    bool oclInit();
    bool save();
    const char *algoName() const;

private:
    Options(int argc, char **argv);
    ~Options();

    inline bool isReady() const { return m_ready; }

    static Options *m_self;

    bool getJSON(const char *fileName, rapidjson::Document &doc);
    bool parseArg(int key, const char *arg);
    bool parseArg(int key, uint64_t arg);
    bool parseBoolean(int key, bool enable);
    Url *parseUrl(const char *arg) const;
    void adjust();
    void parseConfig(const char *fileName);
    void parseJSON(const struct option *option, const rapidjson::Value &object);
    void parseThread(const rapidjson::Value &object);
    void showUsage(int status) const;
    void showVersion(void);

    bool setAlgo(const char *algo);

    bool m_autoConf;
    bool m_background;
    bool m_colors;
    bool m_dryRun;
    bool m_ready;
    bool m_shouldSave;
    bool m_syslog;
    char *m_apiToken;
    char *m_apiWorkerId;
    char *m_configName;
    char *m_logFile;
    char *m_userAgent;
    int m_apiPort;
    int m_donateLevel;
    int m_platformIndex;
    int m_printTime;
    int m_retries;
    int m_retryPause;
    OclCLI m_oclCLI;
    std::vector<OclThread*> m_threads;
    std::vector<Url*> m_pools;
    xmrig::Algo m_algorithm;
};

#endif /* __OPTIONS_H__ */
