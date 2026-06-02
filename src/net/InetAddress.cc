#include "net/InetAddress.h"

#include "base/Logger.h"

#include <arpa/inet.h>

#include <cstring>
#include <sstream>

namespace mini_muduo {

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;

    // loopbackOnly 为 true 时只监听本机 127.0.0.1，否则监听所有网卡 0.0.0.0。
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);

    // inet_pton 将点分十进制字符串转换为网络字节序地址。
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        LOG_FATAL("inet_pton failed for ip: " + ip);
    }
}

std::string InetAddress::toIp() const
{
    char buf[INET_ADDRSTRLEN] = "";
    // inet_ntop 将网络字节序地址转换回点分十进制字符串。
    if (::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)) == nullptr) {
        LOG_ERROR("inet_ntop failed");
        return "";
    }
    return buf;
}

std::string InetAddress::toIpPort() const
{
    std::ostringstream oss;
    oss << toIp() << ":" << ntohs(addr_.sin_port);
    return oss.str();
}

const struct sockaddr* InetAddress::getSockAddr() const
{
    return reinterpret_cast<const struct sockaddr*>(&addr_);
}

void InetAddress::setSockAddr(const struct sockaddr_in& addr)
{
    addr_ = addr;
}

}  // namespace mini_muduo
