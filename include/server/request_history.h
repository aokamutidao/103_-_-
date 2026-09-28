/**
 * @file request_history.h
 * @brief 请求历史记录（用于去重）
 */

#ifndef REQUEST_HISTORY_H
#define REQUEST_HISTORY_H

#include <map>
#include <vector>
#include <cstdint>
#include "common/data_structures.h"

class RequestHistory {
public:
    RequestHistory() = default;
    ~RequestHistory() = default;

    void addEntry(uint32_t request_id, const std::vector<uint8_t>& response);
    bool findEntry(uint32_t request_id, std::vector<uint8_t>& response);
    void cleanup();

private:
    std::map<uint32_t, HistoryEntry> history_;
};

#endif // REQUEST_HISTORY_H
