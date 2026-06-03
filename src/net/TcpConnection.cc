#include "net/TcpConnection.h"

#include "base/Logger.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/Socket.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <sys/socket.h>

#include <cstring>

namespace mini_muduo {

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
}

EventLoop* TcpConnection::getLoop() const
{
    return loop_;
}

const std::string& TcpConnection::name() const
{
    return name_;
}

bool TcpConnection::connected() const
{
    return state_ == kConnected;
}

const InetAddress& TcpConnection::localAddress() const
{
    return localAddr_;
}

const InetAddress& TcpConnection::peerAddress() const
{
    return peerAddr_;
}

void TcpConnection::send(const std::string& message)
{
    send(message.data(), message.size());
}

void TcpConnection::send(const void* data, size_t len)
{
    if (state_ != kConnected) {
        return;
    }

    if (loop_->isInLoopThread()) {
        sendInLoop(data, len);
    } else {
        std::string message(static_cast<const char*>(data), len);
        loop_->runInLoop(std::bind(
            static_cast<void (TcpConnection::*)(const std::string&)>(&TcpConnection::sendInLoop),
            this,
            message));
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback& cb)
{
    closeCallback_ = cb;
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    writeCompleteCallback_ = cb;
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    setState(kConnected);
    channel_->enableReading();

    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();

    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }

    channel_->remove();
}

void TcpConnection::setState(StateE state)
{
    state_ = state;
}

void TcpConnection::handleRead()
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    if (n > 0) {
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_);
        }
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();

    if (channel_->isWriting()) {
        ssize_t n = sockets::write(channel_->fd(),
                                   outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(static_cast<size_t>(n));
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite error: " + std::string(std::strerror(errno)));
        }
    } else {
        LOG_ERROR("TcpConnection fd is down, no more writing");
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    if (connectionCallback_) {
        connectionCallback_(guardThis);
    }
    if (closeCallback_) {
        closeCallback_(guardThis);
    }
}

void TcpConnection::handleError()
{
    LOG_ERROR("TcpConnection " + name_ + " error: " + std::string(std::strerror(errno)));
}

void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();

    if (state_ == kDisconnected) {
        return;
    }

    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - static_cast<size_t>(nwrote);
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                LOG_ERROR("TcpConnection::sendInLoop write error: " +
                          std::string(std::strerror(errno)));
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remaining > 0) {
        const char* bytes = static_cast<const char*>(data) + nwrote;
        outputBuffer_.append(bytes, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();

    if (!channel_->isWriting()) {
        ::shutdown(socket_->fd(), SHUT_WR);
    }
}

}  // namespace mini_muduo
