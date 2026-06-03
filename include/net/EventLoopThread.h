#ifndef MINI_MUDUO_NET_EVENTLOOPTHREAD_H
#define MINI_MUDUO_NET_EVENTLOOPTHREAD_H

#include "base/nocopyable.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace mini_muduo {

class EventLoop;

class EventLoopThread : public nocopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    explicit EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

private:
    EventLoop* loop_;
    bool exiting_;
    ThreadInitCallback callback_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_EVENTLOOPTHREAD_H
