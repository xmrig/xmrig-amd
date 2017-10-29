/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
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

#ifndef __OCLCLI_H__
#define __OCLCLI_H__


#include <vector>


class OclThread;


class OclCLI
{
public:
    OclCLI();

    bool setup(std::vector<OclThread*> &threads);
    void autoConf(std::vector<OclThread*> &threads, int *platformIndex);
    void parseLaunch(const char *arg);

    inline void parseAffinity(const char *arg) { parse(m_affinity, arg); }
    inline void parseDevices(const char *arg)  { parse(m_devices, arg); }

private:
    inline bool isEmpty() const           { return m_devices.empty() && m_intensity.empty(); };
    inline int affinity(int index) const  { return get(m_affinity, index, -1); }
    inline int intensity(int index) const { return get(m_intensity, index, 0); }
    inline int worksize(int index) const  { return get(m_worksize, index, 8); }

    int get(const std::vector<int> &vector, int index, int defaultValue) const;
    void parse(std::vector<int> &vector, const char *arg) const;

    std::vector<int> m_affinity;
    std::vector<int> m_devices;
    std::vector<int> m_intensity;
    std::vector<int> m_worksize;
};


#endif /* __OCLCLI_H__ */
