
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

#ifndef XMRIG_COMMONCONFIG_H
#define XMRIG_COMMONCONFIG_H


#include <vector>


#include "common/interfaces/IConfig.h"
#include "common/net/Pool.h"
#include "common/utils/c_str.h"
#include "common/xmrig.h"


namespace xmrig {


class CommonConfig : public IConfig
{
public:
    CommonConfig();

    inline bool isApiIPv6() const                  { return m_apiIPv6; }
    inline bool isApiRestricted() const            { return m_apiRestricted; }
    inline bool isBackground() const               { return m_background; }
    inline bool isColors() const                   { return m_colors; }
    inline bool isDryRun() const                   { return m_dryRun; }
    inline bool isSyslog() const                   { return m_syslog; }
    inline const char *apiId() const               { return m_apiId.data(); }
    inline const char *apiToken() const            { return m_apiToken.data(); }
    inline const char *apiWorkerId() const         { return m_apiWorkerId.data(); }
    inline const char *logFile() const             { return m_logFile.data(); }
    inline const char *userAgent() const           { return m_userAgent.data(); }
    inline const std::vector<Pool> &pools() const  { return m_activePools; }
    inline int apiPort() const                     { return m_apiPort; }
    inline int donateLevel() const                 { return m_donateLevel; }
    inline int printTime() const                   { return m_printTime; }
    inline int retries() const                     { return m_retries; }
    inline int retryPause() const                  { return m_retryPause; }
    inline int ccUpdateInterval() const            { return m_ccUpdateInterval; }
    inline int ccPort() const                      { return m_ccPort; }
    inline void setColors(bool colors)             { m_colors = colors; }

    inline bool isWatch() const override               { return m_watch && !m_fileName.isNull(); }
    inline bool isDaemonized() const override          { return m_daemonized; }
    inline const Algorithm &algorithm() const override { return m_algorithm; }
    inline const char *fileName() const override       { return m_fileName.data(); }

    bool save() override;

    void printAPI();
    void printPools();
    void printVersions();

protected:
    enum State {
        NoneState,
        ReadyState,
        ErrorState
    };

    bool finalize() override;
    bool parseBoolean(int key, bool enable) override;
    bool parseString(int key, const char *arg) override;
    bool parseUint64(int key, uint64_t arg) override;
    bool parseCCUrl(const char* url) override;
    void setFileName(const char *fileName) override;

    Algorithm m_algorithm;
    bool m_adjusted;
    bool m_apiIPv6;
    bool m_apiRestricted;
    bool m_autoSave;
    bool m_background;
    bool m_colors;
    bool m_dryRun;
    bool m_syslog;
    bool m_watch;
    bool m_daemonized;
    bool m_ccUseTls;
    bool m_ccUseRemoteLogging;
    bool m_ccUploadConfigOnStartup;
    int m_apiPort;
    int m_donateLevel;
    int m_printTime;
    int m_retries;
    int m_retryPause;
    int m_ccUpdateInterval;
    int m_ccPort;
    State m_state;
    std::vector<Pool> m_activePools;
    std::vector<Pool> m_pools;
    xmrig::c_str m_apiId;
    xmrig::c_str m_apiToken;
    xmrig::c_str m_apiWorkerId;
    xmrig::c_str m_fileName;
    xmrig::c_str m_logFile;
    xmrig::c_str m_userAgent;
    xmrig::c_str m_ccUrl;
    xmrig::c_str m_ccHost;
    xmrig::c_str m_ccToken;
    xmrig::c_str m_ccWorkerId;

private:
    bool parseInt(int key, int arg);
};


} /* namespace xmrig */

#endif /* __COMMONCONFIG_H__ */

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

#ifndef XMRIG_COMMONCONFIG_H
#define XMRIG_COMMONCONFIG_H


#include <vector>


#include "common/interfaces/IConfig.h"
#include "common/net/Pool.h"
#include "common/utils/c_str.h"
#include "common/xmrig.h"


namespace xmrig {


class CommonConfig : public IConfig
{
public:
    CommonConfig();

    inline bool isApiIPv6() const                  { return m_apiIPv6; }
    inline bool isApiRestricted() const            { return m_apiRestricted; }
    inline bool isBackground() const               { return m_background; }
    inline bool isColors() const                   { return m_colors; }
    inline bool isDryRun() const                   { return m_dryRun; }
    inline bool isSyslog() const                   { return m_syslog; }
    inline const char *apiId() const               { return m_apiId.data(); }
    inline const char *apiToken() const            { return m_apiToken.data(); }
    inline const char *apiWorkerId() const         { return m_apiWorkerId.data(); }
    inline const char *logFile() const             { return m_logFile.data(); }
    inline const char *userAgent() const           { return m_userAgent.data(); }
    inline bool isApiIPv6() const                  { return m_apiIPv6; }
    inline bool isApiRestricted() const            { return m_apiRestricted; }
    inline bool isAutoSave() const                 { return m_autoSave; }
    inline bool isBackground() const               { return m_background; }
    inline bool isColors() const                   { return m_colors; }
    inline bool isDryRun() const                   { return m_dryRun; }
    inline bool isSyslog() const                   { return m_syslog; }
    inline bool ccUseTls() const                    { return m_ccUseTls; }
    inline bool ccUseRemoteLogging() const          { return m_ccUseRemoteLogging; }
    inline bool ccUploadConfigOnStartup() const     { return m_ccUploadConfigOnStartup; }
    inline const char *apiId() const               { return m_apiId.data(); }
    inline const char *apiToken() const            { return m_apiToken.data(); }
    inline const char *apiWorkerId() const         { return m_apiWorkerId.data(); }
    inline const char *logFile() const             { return m_logFile.data(); }
    inline const char *userAgent() const           { return m_userAgent.data(); }
    inline const char *ccUrl() const                { return m_ccUrl.data(); }
    inline const char *ccHost() const               { return m_ccHost.data(); }
    inline const char *ccToken() const              { return m_ccToken.data(); }
    inline const char *ccWorkerId() const           { return m_ccWorkerId.data(); }

    inline const std::vector<Pool> &pools() const  { return m_activePools; }
    inline int apiPort() const                     { return m_apiPort; }
    inline int donateLevel() const                 { return m_donateLevel; }
    inline int printTime() const                   { return m_printTime; }
    inline int retries() const                     { return m_retries; }
    inline int retryPause() const                  { return m_retryPause; }
    inline int ccUpdateInterval() const            { return m_ccUpdateInterval; }
    inline int ccPort() const                      { return m_ccPort; }
    inline void setColors(bool colors)             { m_colors = colors; }

    inline bool isWatch() const override               { return m_watch && !m_fileName.isNull(); }
    inline bool isDaemonized() const override          { return m_daemonized; }
    inline const Algorithm &algorithm() const override { return m_algorithm; }
    inline const char *fileName() const override       { return m_fileName.data(); }

    bool save() override;

    void printAPI();
    void printPools();
    void printVersions();

protected:
    enum State {
        NoneState,
        ReadyState,
        ErrorState
    };

    bool finalize() override;
    bool parseBoolean(int key, bool enable) override;
    bool parseString(int key, const char *arg) override;
    bool parseUint64(int key, uint64_t arg) override;
    bool parseCCUrl(const char* url) override;
    void setFileName(const char *fileName) override;

    Algorithm m_algorithm;
    bool m_adjusted;
    bool m_apiIPv6;
    bool m_apiRestricted;
    bool m_autoSave;
    bool m_background;
    bool m_colors;
    bool m_dryRun;
    bool m_syslog;
    bool m_watch;
    bool m_daemonized;
    bool m_ccUseTls;
    bool m_ccUseRemoteLogging;
    bool m_ccUploadConfigOnStartup;
    int m_apiPort;
    int m_donateLevel;
    int m_printTime;
    int m_retries;
    int m_retryPause;
    int m_ccUpdateInterval;
    int m_ccPort;
    State m_state;
    std::vector<Pool> m_activePools;
    std::vector<Pool> m_pools;
    xmrig::c_str m_apiId;
    xmrig::c_str m_apiToken;
    xmrig::c_str m_apiWorkerId;
    xmrig::c_str m_fileName;
    xmrig::c_str m_logFile;
    xmrig::c_str m_userAgent;
    xmrig::c_str m_ccUrl;
    xmrig::c_str m_ccHost;
    xmrig::c_str m_ccToken;
    xmrig::c_str m_ccWorkerId;

private:
    bool parseInt(int key, int arg);
};


} /* namespace xmrig */

#endif /* __COMMONCONFIG_H__ */
