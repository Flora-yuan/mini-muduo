#ifndef MINI_MUDUO_NET_TIMERQUEUE_H
#define MINI_MUDUO_NET_TIMERQUEUE_H

#include "base/Timestamp.h"
#include "base/nocopyable.h"
#include "net/Channel.h"
#include "net/Timer.h"
#include "net/TimerId.h"

#include <set>
#include <stdint.h>
#include <vector>

namespace mini_muduo {

class EventLoop;

// TimerQueue 使用 timerfd 将定时事件统一接入 EventLoop。
class TimerQueue : public nocopyable {
public:
    using TimerCallback = Timer::TimerCallback;

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb,
                     Timestamp when,
                     double interval);

    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;

    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);

    void reset(const std::vector<Entry>& expired,
               Timestamp now);

    bool insert(Timer* timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    TimerList timers_;

    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;

    bool callingExpiredTimers_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_TIMERQUEUE_H
