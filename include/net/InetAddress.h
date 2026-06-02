#ifndef MINI_MUDUO_NET_INETADDRESS_H
#define MINI_MUDUO_NET_INETADDRESS_H

#include <netinet/in.h>

#include <cstdint>
#include <string>

namespace mini_muduo {

// InetAddress 负责封装 IPv4 地址 sockaddr_in。
// 当前阶段只处理 AF_INET，后续 TcpServer/TcpConnection 可以直接复用。
class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
    InetAddress(const std::string& ip, uint16_t port);

    std::string toIp() const;
    std::string toIpPort() const;

    const struct sockaddr* getSockAddr() const;
    void setSockAddr(const struct sockaddr_in& addr);

private:
    struct sockaddr_in addr_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_INETADDRESS_H
