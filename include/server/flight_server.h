/**
 * @file flight_server.h
 * @brief 航班服务器
 */

#ifndef FLIGHT_SERVER_H
#define FLIGHT_SERVER_H

#include <cstdint>
#include <atomic>
#include "common/udp_socket.h"
#include "server/flight_manager.h"
#include "server/monitor_manager.h"
#include "server/request_history.h"

class FlightServer {
public:
    FlightServer(uint16_t port);
    ~FlightServer();

    void start();
    void stop();
    void addFlight(const Flight& flight);
    void setAtLeastOnce(bool enable);
    void setLossRate(double rate);

private:
    void requestLoop();
    void handleRequest(const std::vector<uint8_t>& request, struct sockaddr_in& client_addr);
    void sendResponse(const std::vector<uint8_t>& response, const struct sockaddr_in& client_addr);
    bool shouldSimulateLoss();

    uint16_t port_;
    UdpSocket socket_;
    FlightManager flight_manager_;
    MonitorManager monitor_manager_;
    RequestHistory history_;
    std::atomic<bool> running_;
    bool at_least_once_;
    double loss_rate_;
};

#endif // FLIGHT_SERVER_H
