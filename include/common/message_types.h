/**
 * @file message_types.h
 * @brief 消息类型和协议定义
 */

#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <cstdint>

/**
 * @brief 消息类型枚举
 */
enum MessageType : uint32_t {
    // 服务 1：查询航班
    MSG_QUERY_FLIGHT_REQUEST = 1,
    MSG_QUERY_FLIGHT_RESPONSE = 2,

    // 服务 2：查询航班详情
    MSG_GET_FLIGHT_INFO_REQUEST = 3,
    MSG_GET_FLIGHT_INFO_RESPONSE = 4,

    // 服务 3：预订座位
    MSG_RESERVE_SEATS_REQUEST = 5,
    MSG_RESERVE_SEATS_RESPONSE = 6,

    // 服务 4：监控座位更新
    MSG_REGISTER_MONITOR_REQUEST = 7,
    MSG_REGISTER_MONITOR_RESPONSE = 8,
    MSG_CALLBACK_UPDATE = 9,

    // 服务 5：查询剩余座位数
    MSG_GET_SEAT_COUNT_REQUEST = 10,
    MSG_GET_SEAT_COUNT_RESPONSE = 11,

    // 服务 6：取消预订
    MSG_CANCEL_RESERVATION_REQUEST = 12,
    MSG_CANCEL_RESERVATION_RESPONSE = 13,
};

/**
 * @brief 错误码枚举
 */
enum ErrorCode : int32_t {
    ERR_SUCCESS = 0,                    // 成功
    ERR_INVALID_INPUT = 1,              // 无效输入
    ERR_FLIGHT_NOT_FOUND = 2,           // 航班不存在
    ERR_NO_FLIGHTS_FOUND = 3,           // 无匹配航班
    ERR_INSUFFICIENT_SEATS = 4,         // 座位不足
    ERR_INVALID_SEAT_COUNT = 5,         // 无效座位数
    ERR_INVALID_MONITOR_INTERVAL = 6,   // 无效监控时间
    ERR_SERVER_ERROR = 99,              // 服务器内部错误
};

/**
 * @brief 消息头结构（12字节）
 */
struct MessageHeader {
    uint32_t message_type;      // 消息类型
    uint32_t request_id;        // 请求ID
    uint32_t message_length;    // 消息总长度（包括头部）

    static constexpr size_t SIZE = 12;
};

/**
 * @brief 错误码对应的错误信息
 */
inline const char* getErrorMessage(int32_t error_code) {
    switch (error_code) {
        case ERR_SUCCESS: return "Success";
        case ERR_INVALID_INPUT: return "Invalid input";
        case ERR_FLIGHT_NOT_FOUND: return "Flight not found";
        case ERR_NO_FLIGHTS_FOUND: return "No flights found";
        case ERR_INSUFFICIENT_SEATS: return "Insufficient seats";
        case ERR_INVALID_SEAT_COUNT: return "Invalid seat count";
        case ERR_INVALID_MONITOR_INTERVAL: return "Invalid monitor interval";
        case ERR_SERVER_ERROR: return "Server error";
        default: return "Unknown error";
    }
}

#endif // MESSAGE_TYPES_H
