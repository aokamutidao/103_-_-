/**
 * @file data_structures.h
 * @brief 数据结构定义
 */

#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * @brief 时间结构（精确到分钟）
 */
struct Time {
    int32_t year;        // 年
    int32_t month;       // 月
    int32_t day;         // 日
    int32_t hour;        // 时
    int32_t minute;      // 分

    Time() : year(0), month(0), day(0), hour(0), minute(0) {}

    Time(int32_t y, int32_t m, int32_t d, int32_t h, int32_t min)
        : year(y), month(m), day(d), hour(h), minute(min) {}

    /**
     * @brief 转换为字符串
     */
    std::string toString() const {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d",
                 year, month, day, hour, minute);
        return std::string(buffer);
    }

    /**
     * @brief 序列化为字节流（20字节）
     */
    void serialize(uint8_t* buffer) const;

    /**
     * @brief 从字节流反序列化
     */
    void deserialize(const uint8_t* buffer);
};

/**
 * @brief 航班信息结构
 */
struct Flight {
    int32_t flight_id;           // 航班号
    std::string source;          // 出发地
    std::string destination;     // 目的地
    Time departure_time;         // 出发时间
    float airfare;               // 票价
    int32_t seat_availability;   // 可用座位数

    Flight() : flight_id(0), airfare(0.0f), seat_availability(0) {}

    Flight(int32_t id, const std::string& src, const std::string& dest,
           const Time& time, float fare, int32_t seats)
        : flight_id(id), source(src), destination(dest),
          departure_time(time), airfare(fare), seat_availability(seats) {}
};

/**
 * @brief 监控记录
 */
struct MonitorEntry {
    int32_t flight_id;                    // 监控的航班号
    struct sockaddr_in client_addr;       // 客户端地址
    time_t expire_time;                   // 到期时间（Unix时间戳）
    bool active;                          // 是否有效

    MonitorEntry() : flight_id(0), expire_time(0), active(false) {
        memset(&client_addr, 0, sizeof(client_addr));
    }
};

/**
 * @brief 历史记录条目（用于去重）
 */
struct HistoryEntry {
    uint32_t request_id;                  // 请求ID
    std::vector<uint8_t> response_data;   // 响应数据
    time_t timestamp;                     // 时间戳

    HistoryEntry() : request_id(0), timestamp(0) {}
};

#endif // DATA_STRUCTURES_H
