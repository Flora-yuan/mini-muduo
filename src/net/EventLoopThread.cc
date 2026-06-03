#include "net/EventLoopThread.h"

#include "net/EventLoop.h"

namespace mini_muduo {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
    : loop_(nullptr),
      exiting_(false),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;

    EventLoop* loop = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop = loop_;
    }

    if (loop != nullptr) {
        loop->quit();
    }

    if (thread_.joinable()) {
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_ = std::thread(&EventLoopThread::threadFunc, this);

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]() { return loop_ != nullptr; });
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;

    if (callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();

    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

}  // namespace mini_muduo
