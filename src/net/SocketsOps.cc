#include "net/SocketsOps.h"

#include "base/Logger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>

namespace mini_muduo {
namespace sockets {
namespace {

std::string errnoMessage(const std::string& action)
{
    return action + " failed: " + std::strerror(errno);
}

int createSocketOrDie(int flags)
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | flags, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL(errnoMessage("socket"));
    }
    return sockfd;
}

}  // namespace

int createNonblockingOrDie()
{
    // 非阻塞监听 fd 是后续 Reactor 模型需要的默认形态。
    return createSocketOrDie(SOCK_NONBLOCK | SOCK_CLOEXEC);
}

int createBlockingOrDie()
{
    return createSocketOrDie(0);
}

void bindOrDie(int sockfd, const struct sockaddr* addr)
{
    if (::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in))) < 0) {
        LOG_FATAL(errnoMessage("bind"));
    }
}

void listenOrDie(int sockfd)
{
    if (::listen(sockfd, SOMAXCONN) < 0) {
        LOG_FATAL(errnoMessage("listen"));
    }
}

int accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));

#ifdef __linux__
    int connfd = ::accept4(sockfd,
                           reinterpret_cast<struct sockaddr*>(addr),
                           &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        return connfd;
    }
    if (errno != ENOSYS && errno != EINVAL) {
        LOG_ERROR(errnoMessage("accept4"));
        return connfd;
    }
#endif

    // 兼容没有 accept4 的系统：先 accept，再手动设置非阻塞和 close-on-exec。
    connfd = ::accept(sockfd, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
    if (connfd < 0) {
        LOG_ERROR(errnoMessage("accept"));
        return connfd;
    }
    setNonBlockAndCloseOnExec(connfd);
    return connfd;
}

ssize_t read(int sockfd, void* buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t write(int sockfd, const void* buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void close(int sockfd)
{
    if (::close(sockfd) < 0) {
        LOG_ERROR(errnoMessage("close"));
    }
}

void setNonBlockAndCloseOnExec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        LOG_ERROR(errnoMessage("fcntl F_GETFL"));
    } else if (::fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_ERROR(errnoMessage("fcntl F_SETFL O_NONBLOCK"));
    }

    int fdflags = ::fcntl(sockfd, F_GETFD, 0);
    if (fdflags < 0) {
        LOG_ERROR(errnoMessage("fcntl F_GETFD"));
    } else if (::fcntl(sockfd, F_SETFD, fdflags | FD_CLOEXEC) < 0) {
        LOG_ERROR(errnoMessage("fcntl F_SETFD FD_CLOEXEC"));
    }
}

}  // namespace sockets
}  // namespace mini_muduo
