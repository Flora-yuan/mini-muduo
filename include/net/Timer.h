#ifndef MINI_MUDUO_NET_TIMER_H
#define MINI_MUDUO_NET_TIMER_H

#include "base/Timestamp.h"
#include "base/nocopyable.h"

#include <atomic>
#include <functional>
#include <stdint.h>

namespace mini_muduo {

// Timer 保存一个定时任务的回调、到期时间和重复间隔。
class Timer : public nocopyable {
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval);

    // 执行定时器回调。
    void run() const;

    Timestamp expiration() const;
    bool repeat() const;
    int64_t sequence() const;

    // 重复定时器会根据 now 计算下一次到期时间，非重复定时器置为无效。
    void restart(Timestamp now);

    static int64_t numCreated();

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    // 全局递增序号，用于区分不同 Timer 对象。
    static std::atomic<int64_t> s_numCreated_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_TIMER_H
