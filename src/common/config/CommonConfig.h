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

#ifndef __COMMONCONFIG_H__
#define __COMMONCONFIG_H__


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
    ~CommonConfig();

    inline bool isApiIPv6() const                   { return m_apiIPv6; }
    inline bool isApiRestricted() const             { return m_apiRestricted; }
    inline bool isBackground() const                { return m_background; }
    inline bool isColors() const                    { return m_colors; }
    inline bool isDryRun() const                    { return m_dryRun; }
    inline bool isSyslog() const                    { return m_syslog; }
    inline bool ccUseTls() const                    { return m_ccUseTls; }
    inline bool ccUseRemoteLogging() const          { return m_ccUseRemoteLogging; }
    inline const char *apiToken() const             { return m_apiToken.data(); }
    inline const char *apiWorkerId() const          { return m_apiWorkerId.data(); }
    inline const char *logFile() const              { return m_logFile.data(); }
    inline const char *userAgent() const            { return m_userAgent.data(); }
    inline const char *ccHost() const               { return m_ccHost.data(); }
    inline const char *ccToken() const              { return m_ccToken.data(); }
    inline const char *ccWorkerId() const           { return m_ccWorkerId.data(); }
    inline const char *ccAdminUser() const          { return m_ccAdminUser.data(); }
    inline const char *ccAdminPass() const          { return m_ccAdminPass.data(); }
    inline const char *ccClientConfigFolder() const { return m_ccClientConfigFolder.data(); }
    inline const char *ccCustomDashboard() const    { return m_ccCustomDashboard.isNull() ? "index.html" : m_ccCustomDashboard.data(); }
    inline const char *ccKeyFile() const            { return m_ccKeyFile.isNull() ? "server.key" : m_ccKeyFile.data(); }
    inline const char *ccCertFile() const           { return m_ccCertFile.isNull() ? "server.pem" : m_ccCertFile.data(); }

    inline const std::vector<Pool> &pools() const  { return m_activePools; }
    inline int apiPort() const                     { return m_apiPort; }
    inline int donateLevel() const                 { return m_donateLevel; }
    inline int printTime() const                   { return m_printTime; }
    inline int retries() const                     { return m_retries; }
    inline int retryPause() const                  { return m_retryPause; }
    inline int ccUpdateInterval() const             { return m_ccUpdateInterval; }
    inline int ccPort() const                       { return m_ccPort; }
    inline size_t ccRemoteLoggingMaxRows() const    { return m_ccRemoteLoggingMaxRows; }
    inline void setColors(bool colors)             { m_colors = colors; }

    inline bool isWatch() const override               { return m_watch && !m_fileName.isNull(); }
    inline const Algorithm &algorithm() const override { return m_algorithm; }
    inline const char *fileName() const override       { return m_fileName.data(); }

    bool save() override;

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
    void setFileName(const char *fileName) override;

    Algorithm m_algorithm;
    bool m_adjusted;
    bool m_apiIPv6;
    bool m_apiRestricted;
    bool m_background;
    bool m_colors;
    bool m_dryRun;
    bool m_syslog;
    bool m_watch;
    bool m_daemonized;
    bool m_ccUseTls;
    bool m_ccUseRemoteLogging;
    int m_apiPort;
    int m_donateLevel;
    int m_printTime;
    int m_retries;
    int m_retryPause;
    int m_ccUpdateInterval;
    int m_ccPort;
    size_t m_ccRemoteLoggingMaxRows;
    State m_state;
    std::vector<Pool> m_activePools;
    std::vector<Pool> m_pools;
    xmrig::c_str m_apiToken;
    xmrig::c_str m_apiWorkerId;
    xmrig::c_str m_fileName;
    xmrig::c_str m_logFile;
    xmrig::c_str m_userAgent;
    xmrig::c_str m_ccHost;
    xmrig::c_str m_ccToken;
    xmrig::c_str m_ccWorkerId;
    xmrig::c_str m_ccAdminUser;
    xmrig::c_str m_ccAdminPass;
    xmrig::c_str m_ccClientConfigFolder;
    xmrig::c_str m_ccCustomDashboard;
    xmrig::c_str m_ccKeyFile;
    xmrig::c_str m_ccCertFile;

private:
    bool parseInt(int key, int arg);
};


} /* namespace xmrig */

#endif /* __COMMONCONFIG_H__ */
