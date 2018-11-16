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


#include <winsock2.h>
#include <windows.h>


#include "App.h"
#include "core/Controller.h"
#include "core/Config.h"
#include "common/log/Log.h"
#include <algorithm>


void App::background()
{
    if (!m_controller->config()->isBackground()) {
        return;
    }

    HWND hcon = GetConsoleWindow();
    if (hcon) {
        ShowWindow(hcon, SW_HIDE);
    } else {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CloseHandle(h);
        FreeConsole();
    }
}

void App::setMaxTimerResolution()
{
    TIMECAPS tc;
    MMRESULT res = timeGetDevCaps(&tc, sizeof(TIMECAPS));
    if (res == TIMERR_NOERROR) 
    {
        m_timerRes = std::min<unsigned int>(std::max<unsigned int>(tc.wPeriodMin, 1), tc.wPeriodMax);
        res = timeBeginPeriod(m_timerRes);
        if (res == TIMERR_NOERROR)
        {
            LOG_INFO(m_controller->config() ?
                "System timer resolution set to " WHITE_BOLD("%u") " ms" :
                "System timer resolution set to %u ms", m_timerRes);
        }
        else
        {
            LOG_ERR("Failed to set system timer resolution: timeBeginPeriod returned error %u", res);
            m_timerRes = 0;
        }
    }
    else
    {
        LOG_ERR("Failed to set system timer resolution: timeGetDevCaps returned error %u", res);
    }
}

void App::restoreTimerResolution()
{
    if (m_timerRes) {
        timeEndPeriod(m_timerRes); 
    }
}
