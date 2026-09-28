/**
 * @file request_history.cpp
 * @brief 请求历史记录实现
 */

#include "server/request_history.h"
#include "common/logger.h"
#include <ctime>

void RequestHistory::addEntry(uint32_t request_id, const std::vector<uint8_t>& response) {
    HistoryEntry entry;
    entry.request_id = request_id;
    entry.response_data = response;
    entry.timestamp = time(nullptr);

    history_[request_id] = entry;

    LOG_DEBUG("Added history entry for request " + std::to_string(request_id));
}

bool RequestHistory::findEntry(uint32_t request_id, std::vector<uint8_t>& response) {
    auto it = history_.find(request_id);
    if (it == history_.end()) {
        return false;
    }

    response = it->second.response_data;
    LOG_DEBUG("Found history entry for request " + std::to_string(request_id));
    return true;
}

void RequestHistory::cleanup() {
    time_t now = time(nullptr);
    const time_t EXPIRE_TIME = 60; // 60秒过期

    auto it = history_.begin();
    while (it != history_.end()) {
        if (now - it->second.timestamp > EXPIRE_TIME) {
            LOG_DEBUG("Cleaning up history entry " + std::to_string(it->first));
            it = history_.erase(it);
        } else {
            ++it;
        }
    }
}
