#include "net/Acceptor.h"

#include "base/Logger.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

#include <errno.h>

#include <cstring>

namespace mini_muduo {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb)
{
    newConnectionCallback_ = cb;
}

bool Acceptor::listening() const
{
    return listening_;
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);

    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    } else {
        LOG_ERROR("Acceptor::handleRead accept failed: " + std::string(std::strerror(errno)));
    }
}

}  // namespace mini_muduo
