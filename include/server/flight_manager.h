/**
 * @file flight_manager.h
 * @brief 航班管理器
 */

#ifndef FLIGHT_MANAGER_H
#define FLIGHT_MANAGER_H

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include "common/data_structures.h"

/**
 * @brief 航班管理器类
 */
class FlightManager {
public:
    FlightManager() = default;
    ~FlightManager() = default;

    /**
     * @brief 查询航班（按出发地和目的地）
     */
    std::vector<int32_t> queryFlight(const std::string& source, const std::string& destination);

    /**
     * @brief 获取航班详情
     */
    bool getFlightInfo(int32_t flight_id, Flight& flight);

    /**
     * @brief 预订座位
     */
    bool reserveSeats(int32_t flight_id, int32_t seat_count, int32_t& remaining);

    /**
     * @brief 获取座位数
     */
    bool getSeatCount(int32_t flight_id, int32_t& count);

    /**
     * @brief 取消预订
     */
    bool cancelReservation(int32_t flight_id, int32_t seat_count, int32_t& new_count);

    /**
     * @brief 添加航班（测试用）
     */
    void addFlight(const Flight& flight);

    /**
     * @brief 初始化测试数据
     */
    void initTestData();

private:
    std::map<int32_t, Flight> flights_;
    std::mutex mutex_;
};

#endif // FLIGHT_MANAGER_H
