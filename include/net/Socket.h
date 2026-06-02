#ifndef MINI_MUDUO_NET_SOCKET_H
#define MINI_MUDUO_NET_SOCKET_H

#include "base/nocopyable.h"

namespace mini_muduo {

class InetAddress;

// Socket 以 RAII 方式持有 socket fd，确保生命周期结束时自动关闭。
class Socket : public nocopyable {
public:
    explicit Socket(int sockfd);
    ~Socket();

    int fd() const;

    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_SOCKET_H
