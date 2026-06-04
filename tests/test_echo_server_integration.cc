#include "base/Logger.h"
#include "net/Buffer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpServer.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

using namespace mini_muduo;

int main()
{
    LOG_INFO("test_echo_server_integration start");

    EventLoop loop;
    TcpServer server(&loop, InetAddress(9984), "IntegrationEchoServer");
    server.setThreadNum(2);

    std::atomic<bool> echoed(false);

    server.setMessageCallback([](const TcpServer::TcpConnectionPtr& conn, Buffer* buf) {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();
    });

    server.start();

    std::thread clientThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        int clientfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientfd < 0) {
            LOG_ERROR("client socket failed");
            loop.queueInLoop([&loop]() { loop.quit(); });
            return;
        }

        struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9984);
        ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (::connect(clientfd,
                      reinterpret_cast<struct sockaddr*>(&serverAddr),
                      static_cast<socklen_t>(sizeof(serverAddr))) < 0) {
            LOG_ERROR("client connect failed");
            ::close(clientfd);
            loop.queueInLoop([&loop]() { loop.quit(); });
            return;
        }

        const std::string message = "hello integration";
        ssize_t written = ::write(clientfd, message.data(), message.size());
        if (written != static_cast<ssize_t>(message.size())) {
            LOG_ERROR("client write failed");
            ::close(clientfd);
            loop.queueInLoop([&loop]() { loop.quit(); });
            return;
        }

        char buf[1024];
        ssize_t n = ::read(clientfd, buf, sizeof(buf));
        if (n > 0 && std::string(buf, static_cast<size_t>(n)) == message) {
            echoed = true;
        }

        ::close(clientfd);
        loop.queueInLoop([&loop]() { loop.quit(); });
    });

    loop.loop();
    clientThread.join();

    if (echoed) {
        LOG_INFO("test_echo_server_integration success");
        return 0;
    }

    LOG_ERROR("test_echo_server_integration failed");
    return 1;
}
