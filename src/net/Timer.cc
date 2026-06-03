#include "net/Timer.h"

#include <utility>

namespace mini_muduo {

std::atomic<int64_t> Timer::s_numCreated_(0);

Timer::Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(++s_numCreated_)
{
}

void Timer::run() const
{
    callback_();
}

Timestamp Timer::expiration() const
{
    return expiration_;
}

bool Timer::repeat() const
{
    return repeat_;
}

int64_t Timer::sequence() const
{
    return sequence_;
}

void Timer::restart(Timestamp now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

int64_t Timer::numCreated()
{
    return s_numCreated_.load();
}

}  // namespace mini_muduo
