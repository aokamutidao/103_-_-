/**
 * @file message_serializer.h
 * @brief 消息编组/解组
 */

#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <vector>
#include <string>
#include <cstdint>
#include "common/data_structures.h"
#include "common/message_types.h"

/**
 * @brief 消息序列化器
 */
class MessageSerializer {
public:
    // ============================================================
    // 辅助方法
    // ============================================================

    /**
     * @brief 写入消息头
     */
    static void writeHeader(std::vector<uint8_t>& buffer,
                           uint32_t message_type,
                           uint32_t request_id,
                           uint32_t message_length);

    /**
     * @brief 读取消息头
     */
    static void readHeader(const std::vector<uint8_t>& buffer,
                          uint32_t& message_type,
                          uint32_t& request_id,
                          uint32_t& message_length);

    /**
     * @brief 写入32位整数（大端序）
     */
    static void writeUint32(std::vector<uint8_t>& buffer, uint32_t value);

    /**
     * @brief 读取32位整数（大端序）
     */
    static uint32_t readUint32(const std::vector<uint8_t>& buffer, size_t& offset);

    /**
     * @brief 写入字符串（长度前缀）
     */
    static void writeString(std::vector<uint8_t>& buffer, const std::string& str);

    /**
     * @brief 读取字符串（长度前缀）
     */
    static std::string readString(const std::vector<uint8_t>& buffer, size_t& offset);

    /**
     * @brief 写入浮点数
     */
    static void writeFloat(std::vector<uint8_t>& buffer, float value);

    /**
     * @brief 读取浮点数
     */
    static float readFloat(const std::vector<uint8_t>& buffer, size_t& offset);

    // ============================================================
    // 服务1：查询航班
    // ============================================================

    /**
     * @brief 编组查询航班请求
     */
    static void marshalQueryFlightRequest(
        uint32_t request_id,
        const std::string& source,
        const std::string& destination,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组查询航班请求
     */
    static void unmarshalQueryFlightRequest(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        std::string& source,
        std::string& destination
    );

    /**
     * @brief 编组查询航班响应
     */
    static void marshalQueryFlightResponse(
        uint32_t request_id,
        int32_t status_code,
        const std::vector<int32_t>& flight_ids,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组查询航班响应
     */
    static void unmarshalQueryFlightResponse(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& status_code,
        std::vector<int32_t>& flight_ids
    );

    // ============================================================
    // 服务2：查询航班详情
    // ============================================================

    /**
     * @brief 编组查询航班详情请求
     */
    static void marshalGetFlightInfoRequest(
        uint32_t request_id,
        int32_t flight_id,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组查询航班详情请求
     */
    static void unmarshalGetFlightInfoRequest(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& flight_id
    );

    /**
     * @brief 编组查询航班详情响应
     */
    static void marshalGetFlightInfoResponse(
        uint32_t request_id,
        int32_t status_code,
        const Flight& flight,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组查询航班详情响应
     */
    static void unmarshalGetFlightInfoResponse(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& status_code,
        Flight& flight
    );

    // ============================================================
    // 服务3：预订座位
    // ============================================================

    /**
     * @brief 编组预订座位请求
     */
    static void marshalReserveSeatsRequest(
        uint32_t request_id,
        int32_t flight_id,
        int32_t seat_count,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组预订座位请求
     */
    static void unmarshalReserveSeatsRequest(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& flight_id,
        int32_t& seat_count
    );

    /**
     * @brief 编组预订座位响应
     */
    static void marshalReserveSeatsResponse(
        uint32_t request_id,
        int32_t status_code,
        int32_t remaining_seats,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组预订座位响应
     */
    static void unmarshalReserveSeatsResponse(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& status_code,
        int32_t& remaining_seats
    );

    // ============================================================
    // 服务4：注册监控
    // ============================================================

    /**
     * @brief 编组注册监控请求
     */
    static void marshalRegisterMonitorRequest(
        uint32_t request_id,
        int32_t flight_id,
        int32_t interval_seconds,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组注册监控请求
     */
    static void unmarshalRegisterMonitorRequest(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& flight_id,
        int32_t& interval_seconds
    );

    /**
     * @brief 编组注册监控响应
     */
    static void marshalRegisterMonitorResponse(
        uint32_t request_id,
        int32_t status_code,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组注册监控响应
     */
    static void unmarshalRegisterMonitorResponse(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& status_code
    );

    /**
     * @brief 编组回调更新消息
     */
    static void marshalCallbackUpdate(
        int32_t flight_id,
        int32_t new_seat_count,
        int32_t remaining_time,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组回调更新消息
     */
    static void unmarshalCallbackUpdate(
        const std::vector<uint8_t>& buffer,
        int32_t& flight_id,
        int32_t& new_seat_count,
        int32_t& remaining_time
    );

    // ============================================================
    // 服务5：查询座位数
    // ============================================================

    /**
     * @brief 编组查询座位数请求
     */
    static void marshalGetSeatCountRequest(
        uint32_t request_id,
        int32_t flight_id,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组查询座位数请求
     */
    static void unmarshalGetSeatCountRequest(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& flight_id
    );

    /**
     * @brief 编组查询座位数响应
     */
    static void marshalGetSeatCountResponse(
        uint32_t request_id,
        int32_t status_code,
        int32_t seat_count,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组查询座位数响应
     */
    static void unmarshalGetSeatCountResponse(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& status_code,
        int32_t& seat_count
    );

    // ============================================================
    // 服务6：取消预订
    // ============================================================

    /**
     * @brief 编组取消预订请求
     */
    static void marshalCancelReservationRequest(
        uint32_t request_id,
        int32_t flight_id,
        int32_t seat_count,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组取消预订请求
     */
    static void unmarshalCancelReservationRequest(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& flight_id,
        int32_t& seat_count
    );

    /**
     * @brief 编组取消预订响应
     */
    static void marshalCancelReservationResponse(
        uint32_t request_id,
        int32_t status_code,
        int32_t new_seat_count,
        std::vector<uint8_t>& buffer
    );

    /**
     * @brief 解组取消预订响应
     */
    static void unmarshalCancelReservationResponse(
        const std::vector<uint8_t>& buffer,
        uint32_t& request_id,
        int32_t& status_code,
        int32_t& new_seat_count
    );
};

#endif // MESSAGE_SERIALIZER_H
