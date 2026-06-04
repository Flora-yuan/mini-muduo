#include "base/Logger.h"
#include "net/Buffer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpServer.h"

#include <string>

using namespace mini_muduo;

int main()
{
    EventLoop loop;
    TcpServer server(&loop, InetAddress(9002), "DiscardServer");
    server.setThreadNum(3);

    server.setConnectionCallback([](const TcpServer::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO("connection up");
        } else {
            LOG_INFO("connection down");
        }
    });

    server.setMessageCallback([](const TcpServer::TcpConnectionPtr&, Buffer* buf) {
        size_t len = buf->readableBytes();
        buf->retrieveAll();
        LOG_INFO(std::string("discard message length: ") + std::to_string(len));
    });

    server.start();
    loop.loop();

    return 0;
}
