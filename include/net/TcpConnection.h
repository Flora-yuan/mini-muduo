#ifndef MINI_MUDUO_NET_TCPCONNECTION_H
#define MINI_MUDUO_NET_TCPCONNECTION_H

#include "base/nocopyable.h"
#include "net/Buffer.h"
#include "net/InetAddress.h"

#include <functional>
#include <memory>
#include <string>

namespace mini_muduo {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public nocopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    using ConnectionCallback =
        std::function<void(const TcpConnectionPtr&)>;

    using MessageCallback =
        std::function<void(const TcpConnectionPtr&, Buffer*)>;

    using CloseCallback =
        std::function<void(const TcpConnectionPtr&)>;

    using WriteCompleteCallback =
        std::function<void(const TcpConnectionPtr&)>;

    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const;
    const std::string& name() const;
    bool connected() const;

    const InetAddress& localAddress() const;
    const InetAddress& peerAddress() const;

    void send(const std::string& message);
    void send(const void* data, size_t len);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    void connectEstablished();
    void connectDestroyed();

private:
    enum StateE {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    void setState(StateE state);

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void sendInLoop(const void* data, size_t len);

    void shutdownInLoop();

private:
    EventLoop* loop_;
    const std::string name_;
    StateE state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_TCPCONNECTION_H
