#include "net/TimerQueue.h"

#include "base/Logger.h"
#include "net/EventLoop.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <string>
#include <utility>

namespace mini_muduo {
namespace {

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL("timerfd_create failed: " + std::string(std::strerror(errno)));
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / 1000000);
    ts.tv_nsec = static_cast<long>((microseconds % 1000000) * 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany = 0;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    if (n != static_cast<ssize_t>(sizeof(howmany))) {
        LOG_ERROR("TimerQueue::handleRead read failed at " + now.toString()
                  + ": " + std::string(std::strerror(errno)));
    }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    std::memset(&newValue, 0, sizeof(newValue));
    newValue.it_value = howMuchTimeFromNow(expiration);

    if (::timerfd_settime(timerfd, 0, &newValue, NULL) < 0) {
        LOG_ERROR("timerfd_settime failed: " + std::string(std::strerror(errno)));
    }
}

}  // namespace

TimerId::TimerId()
    : timer_(nullptr),
      sequence_(0)
{
}

TimerId::TimerId(Timer* timer, int64_t sequence)
    : timer_(timer),
      sequence_(sequence)
{
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      activeTimers_(),
      cancelingTimers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    sockets::close(timerfd_);

    for (const Entry& timer : timers_) {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();

    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        (void)n;
        delete it->first;
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (const Entry& it : expired) {
        it.second->run();
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry& it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        activeTimers_.erase(timer);
    }

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    for (const Entry& it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat()
            && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it.second->restart(now);
            insert(it.second);
        } else {
            delete it.second;
        }
    }

    if (!timers_.empty()) {
        resetTimerfd(timerfd_, timers_.begin()->second->expiration());
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    timers_.insert(Entry(when, timer));
    activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    return earliestChanged;
}

}  // namespace mini_muduo
