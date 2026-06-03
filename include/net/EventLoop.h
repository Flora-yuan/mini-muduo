#ifndef MINI_MUDUO_NET_EVENTLOOP_H
#define MINI_MUDUO_NET_EVENTLOOP_H

#include "base/Timestamp.h"
#include "base/nocopyable.h"
#include "net/Poller.h"
#include "net/TimerId.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace mini_muduo {

class Channel;
class TimerQueue;

class EventLoop : public nocopyable {
public:
    using Functor = std::function<void()>;
    using TimerCallback = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    bool isInLoopThread() const;
    void assertInLoopThread() const;

private:
    void wakeup();
    void handleRead();
    void doPendingFunctors();

private:
    using ChannelList = Poller::ChannelList;

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    std::atomic<bool> callingPendingFunctors_;
    const std::thread::id threadId_;

    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_EVENTLOOP_H
