/**
 * @file flight_server.cpp
 * @brief 航班服务器实现
 */

#include "server/flight_server.h"
#include "common/message_serializer.h"
#include "common/logger.h"
#include <cstdlib>
#include <ctime>

FlightServer::FlightServer(uint16_t port)
    : port_(port), running_(false), at_least_once_(false), loss_rate_(0.0) {
    srand(static_cast<unsigned>(time(nullptr)));
}

FlightServer::~FlightServer() {
    stop();
}

void FlightServer::start() {
    if (!socket_.bind(port_)) {
        LOG_ERROR("Failed to bind to port " + std::to_string(port_));
        return;
    }

    running_ = true;
    LOG_INFO("Server started on port " + std::to_string(port_));

    flight_manager_.initTestData();

    requestLoop();
}

void FlightServer::stop() {
    running_ = false;
    socket_.close();
    LOG_INFO("Server stopped");
}

void FlightServer::addFlight(const Flight& flight) {
    flight_manager_.addFlight(flight);
}

void FlightServer::setAtLeastOnce(bool enable) {
    at_least_once_ = enable;
    LOG_INFO("At-least-once semantic: " + std::string(enable ? "enabled" : "disabled"));
}

void FlightServer::setLossRate(double rate) {
    loss_rate_ = rate;
    LOG_INFO("Loss rate set to " + std::to_string(rate));
}

void FlightServer::requestLoop() {
    uint8_t buffer[65536];
    struct sockaddr_in client_addr;

    while (running_) {
        ssize_t received = socket_.recvFrom(buffer, sizeof(buffer), client_addr);

        if (received > 0) {
            std::vector<uint8_t> request(buffer, buffer + received);
            handleRequest(request, client_addr);
        }

        // 定期清理过期的监控和历史记录
        monitor_manager_.cleanupExpired();
        history_.cleanup();
    }
}

void FlightServer::handleRequest(const std::vector<uint8_t>& request, struct sockaddr_in& client_addr) {
    if (request.size() < MessageHeader::SIZE) {
        LOG_ERROR("Request too short");
        return;
    }

    // 读取消息类型和请求ID
    uint32_t message_type, request_id, message_length;
    MessageSerializer::readHeader(request, message_type, request_id, message_length);

    LOG_INFO("Received request type=" + std::to_string(message_type) +
             " id=" + std::to_string(request_id));

    // 检查是否是重复请求
    if (at_least_once_) {
        std::vector<uint8_t> cached_response;
        if (history_.findEntry(request_id, cached_response)) {
            LOG_INFO("Duplicate request, returning cached response");
            sendResponse(cached_response, client_addr);
            return;
        }
    }

    // 处理请求
    std::vector<uint8_t> response;

    switch (message_type) {
        case MSG_QUERY_FLIGHT_REQUEST: {
            uint32_t req_id;
            std::string source, destination;
            MessageSerializer::unmarshalQueryFlightRequest(request, req_id, source, destination);

            std::vector<int32_t> flight_ids = flight_manager_.queryFlight(source, destination);

            int32_t status = flight_ids.empty() ? ERR_NO_FLIGHTS_FOUND : ERR_SUCCESS;
            MessageSerializer::marshalQueryFlightResponse(req_id, status, flight_ids, response);
            break;
        }

        case MSG_GET_FLIGHT_INFO_REQUEST: {
            uint32_t req_id;
            int32_t flight_id;
            MessageSerializer::unmarshalGetFlightInfoRequest(request, req_id, flight_id);

            Flight flight;
            bool found = flight_manager_.getFlightInfo(flight_id, flight);

            int32_t status = found ? ERR_SUCCESS : ERR_FLIGHT_NOT_FOUND;
            MessageSerializer::marshalGetFlightInfoResponse(req_id, status, flight, response);
            break;
        }

        case MSG_RESERVE_SEATS_REQUEST: {
            uint32_t req_id;
            int32_t flight_id, seat_count;
            MessageSerializer::unmarshalReserveSeatsRequest(request, req_id, flight_id, seat_count);

            int32_t remaining;
            bool success = flight_manager_.reserveSeats(flight_id, seat_count, remaining);

            int32_t status = success ? ERR_SUCCESS : ERR_INSUFFICIENT_SEATS;
            MessageSerializer::marshalReserveSeatsResponse(req_id, status, remaining, response);

            // 通知监控客户端
            if (success) {
                monitor_manager_.notifyMonitors(flight_id, remaining);
            }
            break;
        }

        case MSG_REGISTER_MONITOR_REQUEST: {
            uint32_t req_id;
            int32_t flight_id, interval_seconds;
            MessageSerializer::unmarshalRegisterMonitorRequest(request, req_id, flight_id, interval_seconds);

            // 检查航班是否存在
            Flight flight;
            bool found = flight_manager_.getFlightInfo(flight_id, flight);

            int32_t status = found ? ERR_SUCCESS : ERR_FLIGHT_NOT_FOUND;
            MessageSerializer::marshalRegisterMonitorResponse(req_id, status, response);

            // 注册监控
            if (found) {
                monitor_manager_.registerMonitor(flight_id, interval_seconds, client_addr);
            }
            break;
        }

        case MSG_GET_SEAT_COUNT_REQUEST: {
            uint32_t req_id;
            int32_t flight_id;
            MessageSerializer::unmarshalGetSeatCountRequest(request, req_id, flight_id);

            int32_t seat_count;
            bool found = flight_manager_.getSeatCount(flight_id, seat_count);

            int32_t status = found ? ERR_SUCCESS : ERR_FLIGHT_NOT_FOUND;
            MessageSerializer::marshalGetSeatCountResponse(req_id, status, seat_count, response);
            break;
        }

        case MSG_CANCEL_RESERVATION_REQUEST: {
            uint32_t req_id;
            int32_t flight_id, seat_count;
            MessageSerializer::unmarshalCancelReservationRequest(request, req_id, flight_id, seat_count);

            int32_t new_count;
            bool success = flight_manager_.cancelReservation(flight_id, seat_count, new_count);

            int32_t status = success ? ERR_SUCCESS : ERR_FLIGHT_NOT_FOUND;
            MessageSerializer::marshalCancelReservationResponse(req_id, status, new_count, response);

            // 通知监控客户端
            if (success) {
                monitor_manager_.notifyMonitors(flight_id, new_count);
            }
            break;
        }

        default:
            LOG_ERROR("Unknown message type: " + std::to_string(message_type));
            return;
    }

    // 记录到历史
    if (at_least_once_ && !response.empty()) {
        history_.addEntry(request_id, response);
    }

    // 发送响应
    sendResponse(response, client_addr);
}

void FlightServer::sendResponse(const std::vector<uint8_t>& response, const struct sockaddr_in& client_addr) {
    if (shouldSimulateLoss()) {
        LOG_WARNING("[Simulated] Response lost");
        return;
    }

    socket_.sendTo(response.data(), response.size(), client_addr);
    LOG_INFO("Sent response, size=" + std::to_string(response.size()));
}

bool FlightServer::shouldSimulateLoss() {
    if (loss_rate_ <= 0.0) return false;
    if (loss_rate_ >= 1.0) return true;

    double random = static_cast<double>(rand()) / RAND_MAX;
    return random < loss_rate_;
}
