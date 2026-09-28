/**
 * @file udp_socket.h
 * @brief UDP Socket 封装
 */

#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * @brief UDP Socket 封装类
 */
class UdpSocket {
public:
    /**
     * @brief 构造函数
     */
    UdpSocket();

    /**
     * @brief 析构函数
     */
    ~UdpSocket();

    /**
     * @brief 绑定端口（服务器用）
     * @param port 端口号
     * @return 成功返回true
     */
    bool bind(uint16_t port);

    /**
     * @brief 发送数据报
     * @param data 数据指针
     * @param length 数据长度
     * @param dest_addr 目标地址
     * @return 发送的字节数，失败返回-1
     */
    ssize_t sendTo(const void* data, size_t length, const struct sockaddr_in& dest_addr);

    /**
     * @brief 接收数据报
     * @param buffer 缓冲区
     * @param buffer_size 缓冲区大小
     * @param src_addr 源地址（输出）
     * @param timeout_seconds 超时时间（秒），0表示不超时
     * @return 接收的字节数，超时返回0，失败返回-1
     */
    ssize_t recvFrom(void* buffer, size_t buffer_size, struct sockaddr_in& src_addr,
                     int timeout_seconds = 0);

    /**
     * @brief 设置超时
     * @param seconds 超时时间（秒）
     */
    void setTimeout(int seconds);

    /**
     * @brief 关闭socket
     */
    void close();

    /**
     * @brief 获取socket文件描述符
     */
    int getSocketFd() const { return sockfd_; }

    /**
     * @brief 是否已绑定
     */
    bool isBound() const { return bound_; }

    /**
     * @brief 创建客户端地址
     * @param ip IP地址字符串
     * @param port 端口号
     * @param addr 输出地址结构
     * @return 成功返回true
     */
    static bool createAddress(const std::string& ip, uint16_t port, struct sockaddr_in& addr);

private:
    int sockfd_;
    bool bound_;

    // 禁止拷贝
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
};

#endif // UDP_SOCKET_H
