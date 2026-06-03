#ifndef MINI_MUDUO_NET_TCPSERVER_H
#define MINI_MUDUO_NET_TCPSERVER_H

#include "base/nocopyable.h"
#include "net/Acceptor.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"

#include <map>
#include <memory>
#include <string>

namespace mini_muduo {

class EventLoop;

class TcpServer : public nocopyable {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = TcpConnection::ConnectionCallback;
    using MessageCallback = TcpConnection::MessageCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;

    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& name);
    ~TcpServer();

    void start();

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool started_;
    int nextConnId_;
    ConnectionMap connections_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_TCPSERVER_H
