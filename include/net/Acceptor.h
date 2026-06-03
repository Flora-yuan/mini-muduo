#ifndef MINI_MUDUO_NET_ACCEPTOR_H
#define MINI_MUDUO_NET_ACCEPTOR_H

#include "base/nocopyable.h"
#include "net/Channel.h"
#include "net/Socket.h"

#include <functional>

namespace mini_muduo {

class EventLoop;
class InetAddress;

class Acceptor : public nocopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& peerAddr)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb);

    bool listening() const;
    void listen();

private:
    void handleRead();

private:
    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_ACCEPTOR_H
