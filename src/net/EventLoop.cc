#include "net/EventLoop.h"

#include "base/Logger.h"
#include "net/Channel.h"
#include "net/EPollPoller.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstring>
#include <utility>

namespace mini_muduo {
namespace {

const int kPollTimeMs = 10000;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("eventfd failed: " + std::string(std::strerror(errno)));
    }
    return evtfd;
}

}  // namespace

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new EPollPoller),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    removeChannel(wakeupChannel_.get());
    sockets::close(wakeupFd_);
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }

        doPendingFunctors();
    }

    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;

    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == std::this_thread::get_id();
}

void EventLoop::assertInLoopThread() const
{
    if (!isInLoopThread()) {
        LOG_FATAL("EventLoop used from a non-loop thread");
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != static_cast<ssize_t>(sizeof(one))) {
        LOG_ERROR("EventLoop::wakeup write failed: " + std::string(std::strerror(errno)));
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 0;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != static_cast<ssize_t>(sizeof(one))) {
        LOG_ERROR("EventLoop::handleRead read failed: " + std::string(std::strerror(errno)));
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}

}  // namespace mini_muduo
