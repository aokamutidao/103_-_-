/**
 * @file udp_socket.cpp
 * @brief UDP Socket 封装实现
 */

#include "common/udp_socket.h"
#include "common/logger.h"
#include <cstring>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

UdpSocket::UdpSocket() : sockfd_(-1), bound_(false) {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        LOG_ERROR("Failed to create socket: " + std::string(strerror(errno)));
    }
}

UdpSocket::~UdpSocket() {
    close();
}

bool UdpSocket::bind(uint16_t port) {
    if (sockfd_ < 0) {
        LOG_ERROR("Invalid socket");
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind to port " + std::to_string(port) + ": " + strerror(errno));
        return false;
    }

    bound_ = true;
    LOG_INFO("Socket bound to port " + std::to_string(port));
    return true;
}

ssize_t UdpSocket::sendTo(const void* data, size_t length, const struct sockaddr_in& dest_addr) {
    if (sockfd_ < 0) {
        LOG_ERROR("Invalid socket");
        return -1;
    }

    ssize_t sent = ::sendto(sockfd_, data, length, 0,
                            (struct sockaddr*)&dest_addr, sizeof(dest_addr));

    if (sent < 0) {
        LOG_ERROR("Failed to send: " + std::string(strerror(errno)));
    }

    return sent;
}

ssize_t UdpSocket::recvFrom(void* buffer, size_t buffer_size, struct sockaddr_in& src_addr,
                            int timeout_seconds) {
    if (sockfd_ < 0) {
        LOG_ERROR("Invalid socket");
        return -1;
    }

    // 设置超时
    if (timeout_seconds > 0) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd_, &readfds);

        struct timeval tv;
        tv.tv_sec = timeout_seconds;
        tv.tv_usec = 0;

        int result = select(sockfd_ + 1, &readfds, nullptr, nullptr, &tv);

        if (result == 0) {
            // 超时
            return 0;
        } else if (result < 0) {
            LOG_ERROR("Select failed: " + std::string(strerror(errno)));
            return -1;
        }
    }

    socklen_t addr_len = sizeof(src_addr);
    ssize_t received = ::recvfrom(sockfd_, buffer, buffer_size, 0,
                                  (struct sockaddr*)&src_addr, &addr_len);

    if (received < 0) {
        LOG_ERROR("Failed to receive: " + std::string(strerror(errno)));
    }

    return received;
}

void UdpSocket::setTimeout(int seconds) {
    if (sockfd_ < 0) {
        return;
    }

    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void UdpSocket::close() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
        bound_ = false;
    }
}

bool UdpSocket::createAddress(const std::string& ip, uint16_t port, struct sockaddr_in& addr) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        LOG_ERROR("Invalid address: " + ip);
        return false;
    }

    return true;
}
