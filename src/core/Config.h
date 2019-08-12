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

#ifndef XMRIG_CONFIG_H
#define XMRIG_CONFIG_H


#include <stdint.h>
#include <vector>


#include "amd/OclCLI.h"
#include "common/config/CommonConfig.h"
#include "common/xmrig.h"
#include "rapidjson/fwd.h"


namespace xmrig {


class ConfigLoader;
class IThread;
class IConfigListener;
class Process;


class Config : public CommonConfig
{
public:
    Config();

    bool isCNv2() const;
    bool oclInit();
    bool reload(const char *json);

    void getJSON(rapidjson::Document &doc) const override;

    inline bool isOclCache() const                       { return m_cache; }
    inline bool isShouldSave() const                     { return m_shouldSave && isAutoSave(); }
    inline const char *loader() const                    { return m_loader.data(); }
    inline const std::vector<IThread *> &threads() const { return m_threads; }
    inline int platformIndex() const                     { return m_platformIndex; }
    inline xmrig::OclVendor vendor() const               { return m_vendor; }

    static Config *load(Process *process, IConfigListener *listener);
    static const char *vendorName(xmrig::OclVendor vendor);

protected:
    bool finalize() override;
    bool parseBoolean(int key, bool enable) override;
    bool parseString(int key, const char *arg) override;
    bool parseUint64(int key, uint64_t arg) override;
    void parseJSON(const rapidjson::Document &doc) override;

private:
    std::vector<IThread *> filterThreads() const;
    void parseThread(const rapidjson::Value &object);
    void setPlatformIndex(const char *name);
    void setPlatformIndex(int index);

    bool m_autoConf;
    bool m_cache;
    bool m_shouldSave;
    int m_platformIndex;
    OclCLI m_oclCLI;
    std::vector<IThread *> m_threads;
    xmrig::String m_loader;
    xmrig::OclVendor m_vendor;
};


} /* namespace xmrig */

#endif /* XMRIG_CONFIG_H */
