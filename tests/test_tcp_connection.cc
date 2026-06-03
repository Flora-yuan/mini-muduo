#include "base/Logger.h"
#include "net/Acceptor.h"
#include "net/Buffer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <memory>
#include <string>
#include <thread>

using namespace mini_muduo;

namespace {

InetAddress localAddressOf(int sockfd)
{
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &addrlen) < 0) {
        LOG_ERROR("getsockname failed");
    }

    InetAddress localAddr;
    localAddr.setSockAddr(addr);
    return localAddr;
}

}  // namespace

int main()
{
    LOG_INFO("test_tcp_connection start");

    EventLoop loop;
    Acceptor acceptor(&loop, InetAddress(9982), true);
    std::shared_ptr<TcpConnection> connection;
    std::atomic<bool> echoed(false);

    acceptor.setNewConnectionCallback(
        [&](int connfd, const InetAddress& peerAddr) {
            InetAddress localAddr = localAddressOf(connfd);

            connection = std::make_shared<TcpConnection>(
                &loop, "test-connection", connfd, localAddr, peerAddr);

            connection->setConnectionCallback(
                [](const TcpConnection::TcpConnectionPtr& conn) {
                    if (conn->connected()) {
                        LOG_INFO("connection established");
                    } else {
                        LOG_INFO("connection disconnected");
                    }
                });

            connection->setMessageCallback(
                [](const TcpConnection::TcpConnectionPtr& conn, Buffer* buffer) {
                    std::string message = buffer->retrieveAllAsString();
                    LOG_INFO("received: " + message);
                    conn->send(message);
                    conn->shutdown();
                });

            connection->setCloseCallback(
                [&](const TcpConnection::TcpConnectionPtr&) {
                    loop.quit();
                });

            connection->connectEstablished();
        });

    acceptor.listen();

    std::thread clientThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        int clientfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientfd < 0) {
            LOG_ERROR("client socket failed");
            loop.quit();
            return;
        }

        struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9982);
        ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (::connect(clientfd,
                      reinterpret_cast<struct sockaddr*>(&serverAddr),
                      static_cast<socklen_t>(sizeof(serverAddr))) < 0) {
            LOG_ERROR("client connect failed");
            ::close(clientfd);
            loop.quit();
            return;
        }

        const std::string message = "hello tcp connection";
        ssize_t written = ::write(clientfd, message.data(), message.size());
        if (written != static_cast<ssize_t>(message.size())) {
            LOG_ERROR("client write failed");
            ::close(clientfd);
            loop.quit();
            return;
        }

        char buf[1024];
        ssize_t n = ::read(clientfd, buf, sizeof(buf));
        if (n > 0 && std::string(buf, static_cast<size_t>(n)) == message) {
            echoed = true;
        }

        ::close(clientfd);
    });

    loop.loop();
    clientThread.join();

    if (echoed) {
        LOG_INFO("test_tcp_connection success");
        return 0;
    }

    LOG_ERROR("test_tcp_connection failed");
    return 1;
}
