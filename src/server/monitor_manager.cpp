/**
 * @file monitor_manager.cpp
 * @brief 监控管理器实现
 */

#include "server/monitor_manager.h"
#include "common/message_serializer.h"
#include "common/logger.h"
#include <ctime>

void MonitorManager::registerMonitor(int32_t flight_id, int32_t interval_seconds, const struct sockaddr_in& client_addr) {
    MonitorEntry entry;
    entry.flight_id = flight_id;
    entry.client_addr = client_addr;
    entry.expire_time = time(nullptr) + interval_seconds;
    entry.active = true;

    monitors_.push_back(entry);

    LOG_INFO("Registered monitor for flight " + std::to_string(flight_id) +
             ", interval: " + std::to_string(interval_seconds) + "s");
}

void MonitorManager::notifyMonitors(int32_t flight_id, int32_t new_seat_count) {
    time_t now = time(nullptr);

    for (auto& monitor : monitors_) {
        if (monitor.active && monitor.flight_id == flight_id && now < monitor.expire_time) {
            // 发送回调
            std::vector<uint8_t> buffer;
            int32_t remaining_time = static_cast<int32_t>(monitor.expire_time - now);

            MessageSerializer::marshalCallbackUpdate(flight_id, new_seat_count, remaining_time, buffer);

            socket_.sendTo(buffer.data(), buffer.size(), monitor.client_addr);

            LOG_INFO("Sent callback to client for flight " + std::to_string(flight_id));
        }
    }
}

void MonitorManager::cleanupExpired() {
    time_t now = time(nullptr);

    auto it = monitors_.begin();
    while (it != monitors_.end()) {
        if (now >= it->expire_time) {
            LOG_INFO("Monitor expired for flight " + std::to_string(it->flight_id));
            it = monitors_.erase(it);
        } else {
            ++it;
        }
    }
}
