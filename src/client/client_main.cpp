/**
 * @file client_main.cpp
 * @brief 客户端主程序
 */

#include "client/flight_client.h"
#include "client/client_ui.h"
#include "common/logger.h"
#include <iostream>
#include <cstring>
#include <memory>

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --server=<ip>         Server IP address (default: 127.0.0.1)\n"
              << "  --port=<port>         Server port (default: 8080)\n"
              << "  --at-least-once       Enable at-least-once semantic\n"
              << "  --loss-rate=<rate>    Message loss rate (0.0-1.0, default: 0.0)\n"
              << "  --help                Show this help\n";
}

int main(int argc, char* argv[]) {
    std::string server_ip = "127.0.0.1";
    uint16_t port = 8080;
    bool at_least_once = false;
    double loss_rate = 0.0;

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--server=", 9) == 0) {
            server_ip = argv[i] + 9;
        } else if (strncmp(argv[i], "--port=", 7) == 0) {
            port = static_cast<uint16_t>(atoi(argv[i] + 7));
        } else if (strcmp(argv[i], "--at-least-once") == 0) {
            at_least_once = true;
        } else if (strncmp(argv[i], "--loss-rate=", 12) == 0) {
            loss_rate = atof(argv[i] + 12);
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }

    LOG_INFO("Starting flight client...");
    LOG_INFO("Server: " + server_ip + ":" + std::to_string(port));
    LOG_INFO("At-least-once: " + std::string(at_least_once ? "enabled" : "disabled"));
    LOG_INFO("Loss rate: " + std::to_string(loss_rate));

    // 创建客户端
    auto client = std::make_shared<FlightClient>(server_ip, port);
    client->setAtLeastOnce(at_least_once);
    client->setLossRate(loss_rate);

    // 创建UI并运行
    ClientUI ui(client);
    ui.run();

    return 0;
}
