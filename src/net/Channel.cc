#include "net/Channel.h"

#include "net/EventLoop.h"

#include <sys/epoll.h>

#include <utility>

namespace mini_muduo {

namespace {

// Channel 初始不关注任何事件。
const uint32_t kNoneEvent = 0;

// 可读事件包括普通数据和带外数据。
const uint32_t kReadEvent = EPOLLIN | EPOLLPRI;

// 可写事件表示 fd 当前可以写入。
const uint32_t kWriteEvent = EPOLLOUT;

// 新建 Channel 还没有加入 Poller。
const int kNew = -1;

}  // namespace

Channel::Channel(int fd)
    : loop_(nullptr),
      fd_(fd),
      events_(kNoneEvent),
      revents_(kNoneEvent),
      index_(kNew)
{
}

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(kNoneEvent),
      revents_(kNoneEvent),
      index_(kNew)
{
}

Channel::~Channel()
{
}

int Channel::fd() const
{
    return fd_;
}

uint32_t Channel::events() const
{
    return events_;
}

uint32_t Channel::revents() const
{
    return revents_;
}

void Channel::set_revents(uint32_t revt)
{
    revents_ = revt;
}

bool Channel::isNoneEvent() const
{
    return events_ == kNoneEvent;
}

void Channel::enableReading()
{
    events_ |= kReadEvent;
    update();
}

void Channel::disableReading()
{
    events_ &= ~kReadEvent;
    update();
}

void Channel::enableWriting()
{
    events_ |= kWriteEvent;
    update();
}

void Channel::disableWriting()
{
    events_ &= ~kWriteEvent;
    update();
}

void Channel::disableAll()
{
    events_ = kNoneEvent;
    update();
}

bool Channel::isWriting() const
{
    return (events_ & kWriteEvent) != 0;
}

bool Channel::isReading() const
{
    return (events_ & kReadEvent) != 0;
}

int Channel::index() const
{
    return index_;
}

void Channel::set_index(int index)
{
    index_ = index;
}

void Channel::setReadCallback(EventCallback cb)
{
    readCallback_ = std::move(cb);
}

void Channel::setWriteCallback(EventCallback cb)
{
    writeCallback_ = std::move(cb);
}

void Channel::setCloseCallback(EventCallback cb)
{
    closeCallback_ = std::move(cb);
}

void Channel::setErrorCallback(EventCallback cb)
{
    errorCallback_ = std::move(cb);
}

void Channel::handleEvent()
{
    handleEventWithGuard();
}

void Channel::remove()
{
    if (loop_ != nullptr) {
        loop_->removeChannel(this);
    }
}

void Channel::update()
{
    if (loop_ != nullptr) {
        loop_->updateChannel(this);
    }
}

void Channel::handleEventWithGuard()
{
    // 对端关闭并且没有可读数据时，优先通知关闭回调。
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) {
            closeCallback_();
        }
    }

    // fd 出错时通知错误回调。
    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }

    // 普通可读、带外数据、对端半关闭都交给读回调处理。
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) {
            readCallback_();
        }
    }

    // 可写事件交给写回调处理。
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}

}  // namespace mini_muduo
