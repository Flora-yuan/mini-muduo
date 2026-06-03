#include "net/EventLoopThreadPool.h"

#include "base/Logger.h"
#include "net/EventLoop.h"

#include <cassert>

namespace mini_muduo {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::setThreadNum(int numThreads)
{
    assert(numThreads >= 0);
    numThreads_ = numThreads;
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        std::unique_ptr<EventLoopThread> thread(new EventLoopThread(cb));
        EventLoop* loop = thread->startLoop();
        threads_.push_back(std::move(thread));
        loops_.push_back(loop);
    }

    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();

    EventLoop* loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (next_ >= static_cast<int>(loops_.size())) {
            next_ = 0;
        }
    }

    return loop;
}

bool EventLoopThreadPool::started() const
{
    return started_;
}

const std::string& EventLoopThreadPool::name() const
{
    return name_;
}

}  // namespace mini_muduo
