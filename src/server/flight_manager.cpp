/**
 * @file flight_manager.cpp
 * @brief 航班管理器实现
 */

#include "server/flight_manager.h"
#include "common/logger.h"

std::vector<int32_t> FlightManager::queryFlight(const std::string& source, const std::string& destination) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<int32_t> result;

    for (const auto& pair : flights_) {
        const Flight& flight = pair.second;
        if (flight.source == source && flight.destination == destination) {
            result.push_back(flight.flight_id);
        }
    }

    LOG_INFO("Query flight: " + source + " -> " + destination + ", found " + std::to_string(result.size()) + " flights");

    return result;
}

bool FlightManager::getFlightInfo(int32_t flight_id, Flight& flight) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = flights_.find(flight_id);
    if (it == flights_.end()) {
        LOG_WARNING("Flight not found: " + std::to_string(flight_id));
        return false;
    }

    flight = it->second;
    return true;
}

bool FlightManager::reserveSeats(int32_t flight_id, int32_t seat_count, int32_t& remaining) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = flights_.find(flight_id);
    if (it == flights_.end()) {
        LOG_WARNING("Flight not found: " + std::to_string(flight_id));
        return false;
    }

    Flight& flight = it->second;

    if (flight.seat_availability < seat_count) {
        LOG_WARNING("Insufficient seats for flight " + std::to_string(flight_id));
        return false;
    }

    flight.seat_availability -= seat_count;
    remaining = flight.seat_availability;

    LOG_INFO("Reserved " + std::to_string(seat_count) + " seats for flight " + std::to_string(flight_id) +
             ", remaining: " + std::to_string(remaining));

    return true;
}

bool FlightManager::getSeatCount(int32_t flight_id, int32_t& count) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = flights_.find(flight_id);
    if (it == flights_.end()) {
        return false;
    }

    count = it->second.seat_availability;
    return true;
}

bool FlightManager::cancelReservation(int32_t flight_id, int32_t seat_count, int32_t& new_count) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = flights_.find(flight_id);
    if (it == flights_.end()) {
        return false;
    }

    Flight& flight = it->second;
    flight.seat_availability += seat_count;
    new_count = flight.seat_availability;

    LOG_INFO("Cancelled " + std::to_string(seat_count) + " seats for flight " + std::to_string(flight_id));

    return true;
}

void FlightManager::addFlight(const Flight& flight) {
    std::lock_guard<std::mutex> lock(mutex_);
    flights_[flight.flight_id] = flight;
    LOG_INFO("Added flight: " + std::to_string(flight.flight_id));
}

void FlightManager::initTestData() {
    addFlight(Flight(1001, "Beijing", "Shanghai", Time(2026, 10, 20, 8, 30), 800.50f, 120));
    addFlight(Flight(1002, "Beijing", "Shanghai", Time(2026, 10, 20, 10, 0), 850.00f, 100));
    addFlight(Flight(1003, "Beijing", "Shanghai", Time(2026, 10, 20, 14, 30), 900.00f, 80));
    addFlight(Flight(2001, "Shanghai", "Beijing", Time(2026, 10, 21, 9, 0), 820.00f, 110));
    addFlight(Flight(2002, "Shanghai", "Beijing", Time(2026, 10, 21, 15, 0), 870.50f, 90));
    addFlight(Flight(3001, "Guangzhou", "Chengdu", Time(2026, 10, 22, 7, 30), 1200.00f, 150));
    addFlight(Flight(3002, "Guangzhou", "Chengdu", Time(2026, 10, 22, 13, 0), 1250.00f, 130));
    addFlight(Flight(4001, "Shenzhen", "Xi'an", Time(2026, 10, 23, 11, 0), 1100.00f, 100));
    addFlight(Flight(5001, "Hangzhou", "Wuhan", Time(2026, 10, 24, 16, 0), 700.00f, 140));
    addFlight(Flight(6001, "Nanjing", "Kunming", Time(2026, 10, 25, 12, 30), 1500.00f, 120));

    LOG_INFO("Initialized " + std::to_string(flights_.size()) + " test flights");
}
