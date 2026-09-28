/**
 * @file message_serializer.cpp
 * @brief 消息编组/解组实现
 */

#include "common/message_serializer.h"
#include "common/logger.h"
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>

// ============================================================
// Time 结构的序列化和反序列化
// ============================================================

void Time::serialize(uint8_t* buffer) const {
    uint32_t* ptr = reinterpret_cast<uint32_t*>(buffer);
    ptr[0] = htonl(year);
    ptr[1] = htonl(month);
    ptr[2] = htonl(day);
    ptr[3] = htonl(hour);
    ptr[4] = htonl(minute);
}

void Time::deserialize(const uint8_t* buffer) {
    const uint32_t* ptr = reinterpret_cast<const uint32_t*>(buffer);
    year = ntohl(ptr[0]);
    month = ntohl(ptr[1]);
    day = ntohl(ptr[2]);
    hour = ntohl(ptr[3]);
    minute = ntohl(ptr[4]);
}

// ============================================================
// 辅助方法实现
// ============================================================

void MessageSerializer::writeHeader(std::vector<uint8_t>& buffer,
                                    uint32_t message_type,
                                    uint32_t request_id,
                                    uint32_t message_length) {
    writeUint32(buffer, message_type);
    writeUint32(buffer, request_id);
    writeUint32(buffer, message_length);
}

void MessageSerializer::readHeader(const std::vector<uint8_t>& buffer,
                                   uint32_t& message_type,
                                   uint32_t& request_id,
                                   uint32_t& message_length) {
    size_t offset = 0;
    message_type = readUint32(buffer, offset);
    request_id = readUint32(buffer, offset);
    message_length = readUint32(buffer, offset);
}

void MessageSerializer::writeUint32(std::vector<uint8_t>& buffer, uint32_t value) {
    uint32_t network_value = htonl(value);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&network_value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(uint32_t));
}

uint32_t MessageSerializer::readUint32(const std::vector<uint8_t>& buffer, size_t& offset) {
    if (offset + sizeof(uint32_t) > buffer.size()) {
        throw std::runtime_error("Buffer underflow in readUint32");
    }

    uint32_t network_value;
    memcpy(&network_value, &buffer[offset], sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return ntohl(network_value);
}

void MessageSerializer::writeString(std::vector<uint8_t>& buffer, const std::string& str) {
    writeUint32(buffer, static_cast<uint32_t>(str.length()));
    buffer.insert(buffer.end(), str.begin(), str.end());
}

std::string MessageSerializer::readString(const std::vector<uint8_t>& buffer, size_t& offset) {
    uint32_t length = readUint32(buffer, offset);

    if (offset + length > buffer.size()) {
        throw std::runtime_error("Buffer underflow in readString");
    }

    std::string str(buffer.begin() + offset, buffer.begin() + offset + length);
    offset += length;

    return str;
}

void MessageSerializer::writeFloat(std::vector<uint8_t>& buffer, float value) {
    uint32_t int_value;
    memcpy(&int_value, &value, sizeof(float));
    writeUint32(buffer, int_value);
}

float MessageSerializer::readFloat(const std::vector<uint8_t>& buffer, size_t& offset) {
    uint32_t int_value = readUint32(buffer, offset);
    float value;
    memcpy(&value, &int_value, sizeof(float));
    return value;
}

// ============================================================
// 服务1：查询航班
// ============================================================

void MessageSerializer::marshalQueryFlightRequest(
    uint32_t request_id,
    const std::string& source,
    const std::string& destination,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    // 先写入消息头（长度稍后更新）
    writeUint32(buffer, MSG_QUERY_FLIGHT_REQUEST);
    writeUint32(buffer, request_id);
    size_t length_pos = buffer.size();
    writeUint32(buffer, 0);  // 占位

    // 写入请求体
    writeString(buffer, source);
    writeString(buffer, destination);

    // 更新消息长度
    uint32_t total_length = static_cast<uint32_t>(buffer.size());
    uint32_t network_length = htonl(total_length);
    memcpy(&buffer[length_pos], &network_length, sizeof(uint32_t));
}

void MessageSerializer::unmarshalQueryFlightRequest(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    std::string& source,
    std::string& destination) {

    size_t offset = 0;

    // 读取消息头
    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    if (message_type != MSG_QUERY_FLIGHT_REQUEST) {
        throw std::runtime_error("Invalid message type");
    }

    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    (void)message_length;  // 消除未使用变量警告

    // 读取请求体
    source = readString(buffer, offset);
    destination = readString(buffer, offset);
}

void MessageSerializer::marshalQueryFlightResponse(
    uint32_t request_id,
    int32_t status_code,
    const std::vector<int32_t>& flight_ids,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    // 消息头
    writeUint32(buffer, MSG_QUERY_FLIGHT_RESPONSE);
    writeUint32(buffer, request_id);
    size_t length_pos = buffer.size();
    writeUint32(buffer, 0);  // 占位

    // 响应体
    writeUint32(buffer, static_cast<uint32_t>(status_code));

    if (status_code == ERR_SUCCESS) {
        writeUint32(buffer, static_cast<uint32_t>(flight_ids.size()));
        for (int32_t id : flight_ids) {
            writeUint32(buffer, static_cast<uint32_t>(id));
        }
    }

    // 更新长度
    uint32_t total_length = static_cast<uint32_t>(buffer.size());
    uint32_t network_length = htonl(total_length);
    memcpy(&buffer[length_pos], &network_length, sizeof(uint32_t));
}

void MessageSerializer::unmarshalQueryFlightResponse(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& status_code,
    std::vector<int32_t>& flight_ids) {

    size_t offset = 0;

    // 消息头
    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    if (message_type != MSG_QUERY_FLIGHT_RESPONSE) {
        throw std::runtime_error("Invalid message type");
    }

    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;

    // 响应体
    status_code = static_cast<int32_t>(readUint32(buffer, offset));

    flight_ids.clear();
    if (status_code == ERR_SUCCESS) {
        uint32_t count = readUint32(buffer, offset);
        for (uint32_t i = 0; i < count; i++) {
            flight_ids.push_back(static_cast<int32_t>(readUint32(buffer, offset)));
        }
    }
}

// ============================================================
// 服务2：查询航班详情
// ============================================================

void MessageSerializer::marshalGetFlightInfoRequest(
    uint32_t request_id,
    int32_t flight_id,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_GET_FLIGHT_INFO_REQUEST);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(flight_id));
}

void MessageSerializer::unmarshalGetFlightInfoRequest(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& flight_id) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    if (message_type != MSG_GET_FLIGHT_INFO_REQUEST) {
        throw std::runtime_error("Invalid message type");
    }

    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    flight_id = static_cast<int32_t>(readUint32(buffer, offset));
}

void MessageSerializer::marshalGetFlightInfoResponse(
    uint32_t request_id,
    int32_t status_code,
    const Flight& flight,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_GET_FLIGHT_INFO_RESPONSE);
    writeUint32(buffer, request_id);
    size_t length_pos = buffer.size();
    writeUint32(buffer, 0);

    writeUint32(buffer, static_cast<uint32_t>(status_code));

    if (status_code == ERR_SUCCESS) {
        writeUint32(buffer, static_cast<uint32_t>(flight.flight_id));
        writeString(buffer, flight.source);
        writeString(buffer, flight.destination);

        // 写入时间（20字节）
        uint8_t time_buffer[20];
        flight.departure_time.serialize(time_buffer);
        buffer.insert(buffer.end(), time_buffer, time_buffer + 20);

        writeFloat(buffer, flight.airfare);
        writeUint32(buffer, static_cast<uint32_t>(flight.seat_availability));
    }

    uint32_t total_length = static_cast<uint32_t>(buffer.size());
    uint32_t network_length = htonl(total_length);
    memcpy(&buffer[length_pos], &network_length, sizeof(uint32_t));
}

void MessageSerializer::unmarshalGetFlightInfoResponse(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& status_code,
    Flight& flight) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    if (message_type != MSG_GET_FLIGHT_INFO_RESPONSE) {
        throw std::runtime_error("Invalid message type");
    }

    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;

    status_code = static_cast<int32_t>(readUint32(buffer, offset));

    if (status_code == ERR_SUCCESS) {
        flight.flight_id = static_cast<int32_t>(readUint32(buffer, offset));
        flight.source = readString(buffer, offset);
        flight.destination = readString(buffer, offset);

        // 读取时间
        if (offset + 20 > buffer.size()) {
            throw std::runtime_error("Buffer underflow in readTime");
        }
        flight.departure_time.deserialize(&buffer[offset]);
        offset += 20;

        flight.airfare = readFloat(buffer, offset);
        flight.seat_availability = static_cast<int32_t>(readUint32(buffer, offset));
    }
}

// 其他服务的实现类似，为了节省空间，这里只列出关键的几个
// 实际开发中需要完整实现所有服务

void MessageSerializer::marshalReserveSeatsRequest(
    uint32_t request_id,
    int32_t flight_id,
    int32_t seat_count,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_RESERVE_SEATS_REQUEST);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + 2 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(flight_id));
    writeUint32(buffer, static_cast<uint32_t>(seat_count));
}

void MessageSerializer::unmarshalReserveSeatsRequest(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& flight_id,
    int32_t& seat_count) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    flight_id = static_cast<int32_t>(readUint32(buffer, offset));
    seat_count = static_cast<int32_t>(readUint32(buffer, offset));
}

void MessageSerializer::marshalReserveSeatsResponse(
    uint32_t request_id,
    int32_t status_code,
    int32_t remaining_seats,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_RESERVE_SEATS_RESPONSE);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + 2 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(status_code));
    writeUint32(buffer, static_cast<uint32_t>(remaining_seats));
}

void MessageSerializer::unmarshalReserveSeatsResponse(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& status_code,
    int32_t& remaining_seats) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    status_code = static_cast<int32_t>(readUint32(buffer, offset));
    remaining_seats = static_cast<int32_t>(readUint32(buffer, offset));
}

// ============================================================
// 服务 4：注册监控
// ============================================================

void MessageSerializer::marshalRegisterMonitorRequest(
    uint32_t request_id,
    int32_t flight_id,
    int32_t interval_seconds,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_REGISTER_MONITOR_REQUEST);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + 2 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(flight_id));
    writeUint32(buffer, static_cast<uint32_t>(interval_seconds));
}

void MessageSerializer::unmarshalRegisterMonitorRequest(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& flight_id,
    int32_t& interval_seconds) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    flight_id = static_cast<int32_t>(readUint32(buffer, offset));
    interval_seconds = static_cast<int32_t>(readUint32(buffer, offset));
}

void MessageSerializer::marshalRegisterMonitorResponse(
    uint32_t request_id,
    int32_t status_code,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_REGISTER_MONITOR_RESPONSE);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(status_code));
}

void MessageSerializer::unmarshalRegisterMonitorResponse(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& status_code) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    status_code = static_cast<int32_t>(readUint32(buffer, offset));
}

void MessageSerializer::marshalCallbackUpdate(
    int32_t flight_id,
    int32_t new_seat_count,
    int32_t remaining_time,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_CALLBACK_UPDATE);
    writeUint32(buffer, 0);  // callback 不需要 request_id
    writeUint32(buffer, MessageHeader::SIZE + 3 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(flight_id));
    writeUint32(buffer, static_cast<uint32_t>(new_seat_count));
    writeUint32(buffer, static_cast<uint32_t>(remaining_time));
}

void MessageSerializer::unmarshalCallbackUpdate(
    const std::vector<uint8_t>& buffer,
    int32_t& flight_id,
    int32_t& new_seat_count,
    int32_t& remaining_time) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    uint32_t request_id = readUint32(buffer, offset);
    (void)request_id;
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    flight_id = static_cast<int32_t>(readUint32(buffer, offset));
    new_seat_count = static_cast<int32_t>(readUint32(buffer, offset));
    remaining_time = static_cast<int32_t>(readUint32(buffer, offset));
}

// ============================================================
// 服务 5：查询座位数
// ============================================================

void MessageSerializer::marshalGetSeatCountRequest(
    uint32_t request_id,
    int32_t flight_id,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_GET_SEAT_COUNT_REQUEST);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(flight_id));
}

void MessageSerializer::unmarshalGetSeatCountRequest(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& flight_id) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    flight_id = static_cast<int32_t>(readUint32(buffer, offset));
}

void MessageSerializer::marshalGetSeatCountResponse(
    uint32_t request_id,
    int32_t status_code,
    int32_t seat_count,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_GET_SEAT_COUNT_RESPONSE);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + 2 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(status_code));
    writeUint32(buffer, static_cast<uint32_t>(seat_count));
}

void MessageSerializer::unmarshalGetSeatCountResponse(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& status_code,
    int32_t& seat_count) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    status_code = static_cast<int32_t>(readUint32(buffer, offset));
    seat_count = static_cast<int32_t>(readUint32(buffer, offset));
}

// ============================================================
// 服务 6：取消预订
// ============================================================

void MessageSerializer::marshalCancelReservationRequest(
    uint32_t request_id,
    int32_t flight_id,
    int32_t seat_count,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_CANCEL_RESERVATION_REQUEST);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + 2 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(flight_id));
    writeUint32(buffer, static_cast<uint32_t>(seat_count));
}

void MessageSerializer::unmarshalCancelReservationRequest(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& flight_id,
    int32_t& seat_count) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    flight_id = static_cast<int32_t>(readUint32(buffer, offset));
    seat_count = static_cast<int32_t>(readUint32(buffer, offset));
}

void MessageSerializer::marshalCancelReservationResponse(
    uint32_t request_id,
    int32_t status_code,
    int32_t new_seat_count,
    std::vector<uint8_t>& buffer) {

    buffer.clear();

    writeUint32(buffer, MSG_CANCEL_RESERVATION_RESPONSE);
    writeUint32(buffer, request_id);
    writeUint32(buffer, MessageHeader::SIZE + 2 * sizeof(uint32_t));
    writeUint32(buffer, static_cast<uint32_t>(status_code));
    writeUint32(buffer, static_cast<uint32_t>(new_seat_count));
}

void MessageSerializer::unmarshalCancelReservationResponse(
    const std::vector<uint8_t>& buffer,
    uint32_t& request_id,
    int32_t& status_code,
    int32_t& new_seat_count) {

    size_t offset = 0;

    uint32_t message_type = readUint32(buffer, offset);
    (void)message_type;
    request_id = readUint32(buffer, offset);
    uint32_t message_length = readUint32(buffer, offset);
    (void)message_length;
    status_code = static_cast<int32_t>(readUint32(buffer, offset));
    new_seat_count = static_cast<int32_t>(readUint32(buffer, offset));
}
