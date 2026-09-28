/**
 * @file flight_client.h
 * @brief 航班客户端
 */

#ifndef FLIGHT_CLIENT_H
#define FLIGHT_CLIENT_H

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include "common/udp_socket.h"
#include "common/data_structures.h"

class FlightClient {
public:
    FlightClient(const std::string& server_ip, uint16_t server_port);
    ~FlightClient();

    // 服务1：查询航班
    int32_t queryFlight(const std::string& source, const std::string& destination,
                       std::vector<int32_t>& flight_ids);

    // 服务2：查询航班详情
    int32_t getFlightInfo(int32_t flight_id, Flight& flight);

    // 服务3：预订座位
    int32_t reserveSeats(int32_t flight_id, int32_t seat_count, int32_t& remaining_seats);

    // 服务4：注册监控（阻塞）
    int32_t registerMonitor(int32_t flight_id, int32_t interval_seconds,
                           std::function<void(int32_t, int32_t, int32_t)> callback);

    // 服务5：查询座位数
    int32_t getSeatCount(int32_t flight_id, int32_t& count);

    // 服务6：取消预订
    int32_t cancelReservation(int32_t flight_id, int32_t seat_count, int32_t& new_count);

    // 设置调用语义
    void setAtLeastOnce(bool enable);

    // 设置丢失率
    void setLossRate(double rate);

private:
    int32_t sendRequest(const std::vector<uint8_t>& request, std::vector<uint8_t>& response);
    int32_t sendRequestWithRetry(const std::vector<uint8_t>& request, std::vector<uint8_t>& response, int max_retries = 3);
    uint32_t generateRequestId();
    bool shouldSimulateLoss();

    std::string server_ip_;
    uint16_t server_port_;
    UdpSocket socket_;
    struct sockaddr_in server_addr_;
    uint32_t request_id_counter_;
    bool at_least_once_;
    double loss_rate_;
};

#endif // FLIGHT_CLIENT_H
