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
    TcpServer server(&loop, InetAddress(9001), "EchoServer");
    server.setThreadNum(3);

    server.setConnectionCallback([](const TcpServer::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO("connection up");
        } else {
            LOG_INFO("connection down");
        }
    });

    server.setMessageCallback([](const TcpServer::TcpConnectionPtr& conn, Buffer* buf) {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
    });

    server.start();
    loop.loop();

    return 0;
}
