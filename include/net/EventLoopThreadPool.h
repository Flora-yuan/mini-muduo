#ifndef MINI_MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MINI_MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include "base/nocopyable.h"
#include "net/EventLoopThread.h"

#include <memory>
#include <string>
#include <vector>

namespace mini_muduo {

class EventLoop;

class EventLoopThreadPool : public nocopyable {
public:
    using ThreadInitCallback = EventLoopThread::ThreadInitCallback;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads);
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();

    bool started() const;
    const std::string& name() const;

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_EVENTLOOPTHREADPOOL_H
