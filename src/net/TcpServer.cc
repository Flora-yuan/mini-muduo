#include "net/TcpServer.h"

#include "base/Logger.h"
#include "net/EventLoop.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <functional>

namespace mini_muduo {

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

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string& name)
    : loop_(loop),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr, true)),
      connectionCallback_([](const TcpConnectionPtr&) {}),
      messageCallback_([](const TcpConnectionPtr&, Buffer*) {}),
      writeCompleteCallback_([](const TcpConnectionPtr&) {}),
      started_(false),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    if (!started_) {
        started_ = true;
        loop_->runInLoop([this]() {
            acceptor_->listen();
        });
    }
}

void TcpServer::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

void TcpServer::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    writeCompleteCallback_ = cb;
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();

    std::string connName = name_ + "-" + std::to_string(nextConnId_++);
    InetAddress localAddr = localAddressOf(sockfd);

    TcpConnectionPtr conn(new TcpConnection(
        loop_,
        connName,
        sockfd,
        localAddr,
        peerAddr));

    connections_[connName] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection,
                  this,
                  std::placeholders::_1));

    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop,
                  this,
                  conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();

    connections_.erase(conn->name());
    conn->connectDestroyed();
}

}  // namespace mini_muduo
