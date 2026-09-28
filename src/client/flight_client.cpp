/**
 * @file flight_client.cpp
 * @brief 航班客户端实现
 */

#include "client/flight_client.h"
#include "common/message_serializer.h"
#include "common/logger.h"
#include <cstdlib>
#include <ctime>

FlightClient::FlightClient(const std::string& server_ip, uint16_t server_port)
    : server_ip_(server_ip), server_port_(server_port), request_id_counter_(0),
      at_least_once_(false), loss_rate_(0.0) {

    srand(static_cast<unsigned>(time(nullptr)));

    if (!UdpSocket::createAddress(server_ip, server_port, server_addr_)) {
        LOG_ERROR("Failed to create server address");
    }
}

FlightClient::~FlightClient() {
    socket_.close();
}

int32_t FlightClient::queryFlight(const std::string& source, const std::string& destination,
                                  std::vector<int32_t>& flight_ids) {
    uint32_t request_id = generateRequestId();
    std::vector<uint8_t> request, response;

    MessageSerializer::marshalQueryFlightRequest(request_id, source, destination, request);

    int32_t result = at_least_once_ ? sendRequestWithRetry(request, response) : sendRequest(request, response);

    if (result == ERR_SUCCESS) {
        uint32_t resp_id;
        int32_t status_code;
        MessageSerializer::unmarshalQueryFlightResponse(response, resp_id, status_code, flight_ids);
        return status_code;
    }

    return result;
}

int32_t FlightClient::getFlightInfo(int32_t flight_id, Flight& flight) {
    uint32_t request_id = generateRequestId();
    std::vector<uint8_t> request, response;

    MessageSerializer::marshalGetFlightInfoRequest(request_id, flight_id, request);

    int32_t result = at_least_once_ ? sendRequestWithRetry(request, response) : sendRequest(request, response);

    if (result == ERR_SUCCESS) {
        uint32_t resp_id;
        int32_t status_code;
        MessageSerializer::unmarshalGetFlightInfoResponse(response, resp_id, status_code, flight);
        return status_code;
    }

    return result;
}

int32_t FlightClient::reserveSeats(int32_t flight_id, int32_t seat_count, int32_t& remaining_seats) {
    uint32_t request_id = generateRequestId();
    std::vector<uint8_t> request, response;

    MessageSerializer::marshalReserveSeatsRequest(request_id, flight_id, seat_count, request);

    int32_t result = at_least_once_ ? sendRequestWithRetry(request, response) : sendRequest(request, response);

    if (result == ERR_SUCCESS) {
        uint32_t resp_id;
        int32_t status_code;
        MessageSerializer::unmarshalReserveSeatsResponse(response, resp_id, status_code, remaining_seats);
        return status_code;
    }

    return result;
}

int32_t FlightClient::registerMonitor(int32_t flight_id, int32_t interval_seconds,
                                      std::function<void(int32_t, int32_t, int32_t)> callback) {
    uint32_t request_id = generateRequestId();
    std::vector<uint8_t> request, response;

    MessageSerializer::marshalRegisterMonitorRequest(request_id, flight_id, interval_seconds, request);

    int32_t result = at_least_once_ ? sendRequestWithRetry(request, response) : sendRequest(request, response);

    if (result != ERR_SUCCESS) {
        return result;
    }

    // 解析注册响应
    uint32_t resp_id;
    int32_t status_code;
    MessageSerializer::unmarshalRegisterMonitorResponse(response, resp_id, status_code);

    if (status_code != ERR_SUCCESS) {
        return status_code;
    }

    LOG_INFO("Monitor registered, entering callback receive loop");

    // 进入阻塞循环接收回调
    uint8_t buffer[65536];
    struct sockaddr_in src_addr;

    // 设置较长的超时时间
    socket_.setTimeout(interval_seconds + 5);

    time_t start_time = time(nullptr);
    time_t end_time = start_time + interval_seconds;

    while (time(nullptr) < end_time) {
        ssize_t received = socket_.recvFrom(buffer, sizeof(buffer), src_addr, interval_seconds + 5);

        if (received <= 0) {
            LOG_WARNING("Callback receive timeout or error");
            break;
        }

        std::vector<uint8_t> msg(buffer, buffer + received);

        // 检查消息类型
        uint32_t msg_type, req_id, msg_len;
        MessageSerializer::readHeader(msg, msg_type, req_id, msg_len);

        if (msg_type == MSG_CALLBACK_UPDATE) {
            int32_t cb_flight_id, new_seat_count, remaining_time;
            MessageSerializer::unmarshalCallbackUpdate(msg, cb_flight_id, new_seat_count, remaining_time);

            LOG_INFO("Received callback: flight=" + std::to_string(cb_flight_id) +
                     " seats=" + std::to_string(new_seat_count) +
                     " remaining=" + std::to_string(remaining_time) + "s");

            // 调用用户回调
            callback(cb_flight_id, new_seat_count, remaining_time);

            // 如果剩余时间为 0，退出循环
            if (remaining_time <= 0) {
                LOG_INFO("Monitor time expired");
                break;
            }
        } else {
            LOG_WARNING("Received unexpected message type: " + std::to_string(msg_type));
        }
    }

    return ERR_SUCCESS;
}

int32_t FlightClient::getSeatCount(int32_t flight_id, int32_t& count) {
    uint32_t request_id = generateRequestId();
    std::vector<uint8_t> request, response;

    MessageSerializer::marshalGetSeatCountRequest(request_id, flight_id, request);

    int32_t result = at_least_once_ ? sendRequestWithRetry(request, response) : sendRequest(request, response);

    if (result == ERR_SUCCESS) {
        uint32_t resp_id;
        int32_t status_code;
        MessageSerializer::unmarshalGetSeatCountResponse(response, resp_id, status_code, count);
        return status_code;
    }

    return result;
}

int32_t FlightClient::cancelReservation(int32_t flight_id, int32_t seat_count, int32_t& new_count) {
    uint32_t request_id = generateRequestId();
    std::vector<uint8_t> request, response;

    MessageSerializer::marshalCancelReservationRequest(request_id, flight_id, seat_count, request);

    int32_t result = at_least_once_ ? sendRequestWithRetry(request, response) : sendRequest(request, response);

    if (result == ERR_SUCCESS) {
        uint32_t resp_id;
        int32_t status_code;
        MessageSerializer::unmarshalCancelReservationResponse(response, resp_id, status_code, new_count);
        return status_code;
    }

    return result;
}

void FlightClient::setAtLeastOnce(bool enable) {
    at_least_once_ = enable;
    LOG_INFO("At-least-once semantic: " + std::string(enable ? "enabled" : "disabled"));
}

void FlightClient::setLossRate(double rate) {
    loss_rate_ = rate;
    LOG_INFO("Loss rate set to " + std::to_string(rate));
}

int32_t FlightClient::sendRequest(const std::vector<uint8_t>& request, std::vector<uint8_t>& response) {
    if (shouldSimulateLoss()) {
        LOG_WARNING("[Simulated] Request lost");
        return ERR_SERVER_ERROR;
    }

    socket_.sendTo(request.data(), request.size(), server_addr_);

    uint8_t buffer[65536];
    struct sockaddr_in src_addr;

    socket_.setTimeout(5);
    ssize_t received = socket_.recvFrom(buffer, sizeof(buffer), src_addr, 5);

    if (received <= 0) {
        LOG_ERROR("Receive timeout or error");
        return ERR_SERVER_ERROR;
    }

    response.assign(buffer, buffer + received);
    return ERR_SUCCESS;
}

int32_t FlightClient::sendRequestWithRetry(const std::vector<uint8_t>& request,
                                           std::vector<uint8_t>& response, int max_retries) {
    for (int i = 0; i < max_retries; i++) {
        int32_t result = sendRequest(request, response);
        if (result == ERR_SUCCESS) {
            return ERR_SUCCESS;
        }
        LOG_WARNING("Request failed, retry " + std::to_string(i + 1) + "/" + std::to_string(max_retries));
    }
    return ERR_SERVER_ERROR;
}

uint32_t FlightClient::generateRequestId() {
    return ++request_id_counter_;
}

bool FlightClient::shouldSimulateLoss() {
    if (loss_rate_ <= 0.0) return false;
    if (loss_rate_ >= 1.0) return true;

    double random = static_cast<double>(rand()) / RAND_MAX;
    return random < loss_rate_;
}
