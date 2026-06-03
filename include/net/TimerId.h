#ifndef MINI_MUDUO_NET_TIMERID_H
#define MINI_MUDUO_NET_TIMERID_H

#include <stdint.h>

namespace mini_muduo {

class Timer;

// TimerId 是暴露给用户的定时器句柄，用于后续取消定时器。
class TimerId {
public:
    TimerId();
    TimerId(Timer* timer, int64_t sequence);

private:
    friend class TimerQueue;

    // TimerQueue 内部通过裸指针和序号精确定位一个定时器。
    Timer* timer_;
    int64_t sequence_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_TIMERID_H
