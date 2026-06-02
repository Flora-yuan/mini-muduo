#include "base/Logger.h"
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <string>

namespace {

void setBlocking(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        LOG_ERROR(std::string("fcntl F_GETFL failed: ") + std::strerror(errno));
        return;
    }
    if (::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        LOG_ERROR(std::string("fcntl F_SETFL blocking failed: ") + std::strerror(errno));
    }
}

void writeAll(int fd, const char* data, ssize_t len)
{
    ssize_t written = 0;
    while (written < len) {
        ssize_t n = mini_muduo::sockets::write(fd, data + written, static_cast<size_t>(len - written));
        if (n > 0) {
            written += n;
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else {
            LOG_ERROR(std::string("write failed: ") + std::strerror(errno));
            break;
        }
    }
}

}  // namespace

int main()
{
    LOG_INFO("blocking echo server start at 0.0.0.0:9000");

    mini_muduo::Socket listenSocket(mini_muduo::sockets::createBlockingOrDie());
    listenSocket.setReuseAddr(true);
    listenSocket.setReusePort(true);

    mini_muduo::InetAddress listenAddress(9000);
    listenSocket.bindAddress(listenAddress);
    listenSocket.listen();

    while (true) {
        mini_muduo::InetAddress peerAddress;
        int connfd = listenSocket.accept(&peerAddress);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR(std::string("accept failed: ") + std::strerror(errno));
            continue;
        }

        // sockets::accept 返回非阻塞 fd；这里转回阻塞模式，保持示例简单直观。
        setBlocking(connfd);
        mini_muduo::Socket connSocket(connfd);

        LOG_INFO("client connected: " + peerAddress.toIpPort());

        char buf[4096];
        while (true) {
            ssize_t n = mini_muduo::sockets::read(connSocket.fd(), buf, sizeof(buf));
            if (n > 0) {
                LOG_INFO("received message from " + peerAddress.toIpPort());
                writeAll(connSocket.fd(), buf, n);
            } else if (n == 0) {
                LOG_INFO("client disconnected: " + peerAddress.toIpPort());
                break;
            } else if (errno == EINTR) {
                continue;
            } else {
                LOG_ERROR(std::string("read failed: ") + std::strerror(errno));
                break;
            }
        }
    }

    return 0;
}
