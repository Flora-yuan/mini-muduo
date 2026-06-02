#include "net/Socket.h"

#include "base/Logger.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include <cstring>

namespace mini_muduo {
namespace {

void setSocketOption(int sockfd, int level, int option, bool on, const char* optionName)
{
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd, level, option, &optval, static_cast<socklen_t>(sizeof(optval))) < 0) {
        LOG_ERROR(std::string("setsockopt ") + optionName + " failed: " + std::strerror(errno));
    }
}

}  // namespace

Socket::Socket(int sockfd)
    : sockfd_(sockfd)
{
}

Socket::~Socket()
{
    sockets::close(sockfd_);
}

int Socket::fd() const
{
    return sockfd_;
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    sockets::bindOrDie(sockfd_, localaddr.getSockAddr());
}

void Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0 && peeraddr != nullptr) {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::setTcpNoDelay(bool on)
{
    // TCP_NODELAY 关闭 Nagle 算法，适合低延迟小包发送。
    setSocketOption(sockfd_, IPPROTO_TCP, TCP_NODELAY, on, "TCP_NODELAY");
}

void Socket::setReuseAddr(bool on)
{
    setSocketOption(sockfd_, SOL_SOCKET, SO_REUSEADDR, on, "SO_REUSEADDR");
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    setSocketOption(sockfd_, SOL_SOCKET, SO_REUSEPORT, on, "SO_REUSEPORT");
#else
    (void)on;
    LOG_ERROR("SO_REUSEPORT is not supported on this system");
#endif
}

void Socket::setKeepAlive(bool on)
{
    setSocketOption(sockfd_, SOL_SOCKET, SO_KEEPALIVE, on, "SO_KEEPALIVE");
}

}  // namespace mini_muduo
