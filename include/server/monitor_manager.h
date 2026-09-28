/**
 * @file monitor_manager.h
 * @brief 监控管理器
 */

#ifndef MONITOR_MANAGER_H
#define MONITOR_MANAGER_H

#include <vector>
#include <cstdint>
#include "common/data_structures.h"
#include "common/udp_socket.h"

class MonitorManager {
public:
    MonitorManager() = default;
    ~MonitorManager() = default;

    void registerMonitor(int32_t flight_id, int32_t interval_seconds, const struct sockaddr_in& client_addr);
    void notifyMonitors(int32_t flight_id, int32_t new_seat_count);
    void cleanupExpired();

private:
    std::vector<MonitorEntry> monitors_;
    UdpSocket socket_;
};

#endif // MONITOR_MANAGER_H
