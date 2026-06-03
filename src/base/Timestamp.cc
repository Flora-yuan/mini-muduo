#include "base/Timestamp.h"

#include <stdio.h>
#include <sys/time.h>

namespace mini_muduo {

namespace {

const int kMicroSecondsPerSecond = 1000 * 1000;

}  // namespace

Timestamp::Timestamp()
    : microSecondsSinceEpoch_(0)
{
}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}

bool Timestamp::valid() const
{
    return microSecondsSinceEpoch_ > 0;
}

int64_t Timestamp::microSecondsSinceEpoch() const
{
    return microSecondsSinceEpoch_;
}

time_t Timestamp::secondsSinceEpoch() const
{
    return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
}

std::string Timestamp::toString() const
{
    char buf[32] = {0};
    const int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    const int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%lld.%06lld",
             static_cast<long long>(seconds),
             static_cast<long long>(microseconds));
    return buf;
}

std::string Timestamp::toFormattedString() const
{
    char buf[32] = {0};
    const time_t seconds = secondsSinceEpoch();
    struct tm tm_time;

#if defined(_WIN32)
    localtime_s(&tm_time, &seconds);
#else
    localtime_r(&seconds, &tm_time);
#endif

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_time);
    return buf;
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid()
{
    return Timestamp();
}

bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * 1000000);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace mini_muduo
