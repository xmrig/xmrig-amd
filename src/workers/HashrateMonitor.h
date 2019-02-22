/* XMRigCC
 * Copyright 2018-     BenDr0id    <ben@graef.in>
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

#ifndef __HASHRATE_MONITOR_H__
#define __HASHRATE_MONITOR_H__

#include <uv.h>
#include <chrono>
#include <ctime>
#include <list>

#include "core/Controller.h"

class Hashrate;
class NetworkState;

#define TIMER_INTERVAL 10000
#define MINUTE_IN_MS 60000

class HashrateMonitor
{
public:
    HashrateMonitor(uv_async_t* async, xmrig::Controller *controller);
    ~HashrateMonitor();

    static void updateHashrate(const Hashrate *hashrate, bool isDonation);
    static void updateNetworkState(const xmrig::NetworkState& network);

private:
    static void onThreadStarted(void *handle);
    static void onTick(uv_timer_t *handle);

    static HashrateMonitor* m_self;
    static uv_mutex_t m_mutex;

    int m_connectionTime;
    std::time_t m_lastDonationTime;
    std::list<std::pair<double, double>> m_hashrates;

    uv_async_t* m_async;
    uv_timer_t m_timer;
    uv_loop_t m_monitor_loop;
    uv_thread_t m_thread;

    xmrig::Controller *m_controller;
};

#endif /* __HASHRATE_MONITOR_H__ */
