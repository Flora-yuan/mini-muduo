#include "base/Logger.h"
#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

int main()
{
    LOG_INFO("test_acceptor start");

    mini_muduo::EventLoop loop;
    mini_muduo::InetAddress listenAddr(9981);
    mini_muduo::Acceptor acceptor(&loop, listenAddr, true);
    std::atomic<bool> accepted(false);

    acceptor.setNewConnectionCallback(
        [&](int sockfd, const mini_muduo::InetAddress& peerAddr) {
            LOG_INFO(std::string("new connection from ") + peerAddr.toIpPort());
            accepted = true;
            mini_muduo::sockets::close(sockfd);
            loop.quit();
        });

    acceptor.listen();

    std::thread clientThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        int clientfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (clientfd < 0) {
            LOG_ERROR("client socket failed");
            return;
        }

        struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof serverAddr);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9981);
        ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        int ret = ::connect(clientfd,
                            reinterpret_cast<struct sockaddr*>(&serverAddr),
                            sizeof serverAddr);
        if (ret < 0) {
            LOG_ERROR("client connect failed");
        }

        ::close(clientfd);
    });

    loop.loop();
    clientThread.join();

    if (accepted) {
        LOG_INFO("test_acceptor success");
        return 0;
    }

    LOG_ERROR("test_acceptor failed");
    return 1;
}
