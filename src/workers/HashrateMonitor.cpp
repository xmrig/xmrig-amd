/* XMRigCC
 * Copyright 2019-     BenDr0id    <ben@graef.in>
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

#include <cstring>
#include <sstream>
#include <math.h>

#include "common/log/Log.h"
#include "cc/ControlCommand.h"
#include "api/NetworkState.h"
#include "HashrateMonitor.h"
#include "App.h"

#include "core/Config.h"

#include "workers/Workers.h"
#include "workers/Hashrate.h"

HashrateMonitor* HashrateMonitor::m_self = nullptr;
uv_mutex_t HashrateMonitor::m_mutex;

HashrateMonitor::HashrateMonitor(uv_async_t* async, xmrig::Controller *controller)
        : m_connectionTime(0),
          m_lastDonationTime(0),
          m_async(async),
          m_controller(controller)
{
    uv_mutex_init(&m_mutex);

    m_self = this;

    uv_thread_create(&m_thread, HashrateMonitor::onThreadStarted, this);
}

HashrateMonitor::~HashrateMonitor()
{
    uv_timer_stop(&m_timer);
    m_self = nullptr;
}

void HashrateMonitor::updateHashrate(const Hashrate* hashrate, bool isDonation)
{
    if (m_self){
        uv_mutex_lock(&m_mutex);

        if (!isDonation) {
            m_self->m_hashrates.clear();

            for (size_t i=0; i < hashrate->threads(); i++) {
                m_self->m_hashrates.emplace_back(hashrate->calc(i, TIMER_INTERVAL), hashrate->calc(i, MINUTE_IN_MS));
            }
        } else {
            auto time_point = std::chrono::system_clock::now();
            m_self->m_lastDonationTime = static_cast<uint64_t>(std::chrono::system_clock::to_time_t(time_point));
        }

        uv_mutex_unlock(&m_mutex);
    }
}

void HashrateMonitor::updateNetworkState(const xmrig::NetworkState& network)
{
    if (m_self){
        uv_mutex_lock(&m_mutex);
        m_self->m_connectionTime = network.connectionTime();
        uv_mutex_unlock(&m_mutex);
    }
}

void HashrateMonitor::onThreadStarted(void* handle)
{
    if (m_self) {
        uv_loop_init(&m_self->m_monitor_loop);

        uv_timer_init(&m_self->m_monitor_loop, &m_self->m_timer);
        uv_timer_start(&m_self->m_timer, HashrateMonitor::onTick,
                       static_cast<uint64_t>(TIMER_INTERVAL),
                       static_cast<uint64_t>(TIMER_INTERVAL));

        uv_run(&m_self->m_monitor_loop, UV_RUN_DEFAULT);
    }
}

void HashrateMonitor::onTick(uv_timer_t* handle)
{
    if (m_self) {
        if (!m_self->m_hashrates.empty()) {
            if (m_self->m_connectionTime > 120) {
                bool error = false;
                double totalHashrate = 0;
                size_t threadId = 0;

                for (auto hashrate : m_self->m_hashrates) {
                    if (isnan(hashrate.first) && isnan(hashrate.second)) {
                        LOG_WARN("[WATCHDOG] GPU Thread #%d crash detected!", threadId);
                        error = true;
                    } else {
                        totalHashrate += hashrate.second;
                    }

                    threadId++;
                }

                if (totalHashrate < m_self->m_controller->config()->minRigHashrate()) {
                    auto time_point = std::chrono::system_clock::now();
                    auto now = std::chrono::system_clock::to_time_t(time_point);

                    if ((now - m_self->m_lastDonationTime) > (MINUTE_IN_MS + TIMER_INTERVAL)/1000) {
                        LOG_WARN("[WATCHDOG] Total hashrate (%.0f) is below minimum (%d)!", totalHashrate,
                                 m_self->m_controller->config()->minRigHashrate());
                        error = true;
                    }
                }

                if (error) {
                    if (m_self->m_controller->config()->isRebootOnCardCrash()) {
                        m_self->m_async->data = reinterpret_cast<void*>(ControlCommand::REBOOT);
                    } else {
                        m_self->m_async->data = reinterpret_cast<void*>(ControlCommand::RESTART);
                    }

                    uv_async_send(m_self->m_async);
                }
            }
        }
    }
}
