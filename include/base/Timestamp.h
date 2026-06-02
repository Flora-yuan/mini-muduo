#ifndef MINI_MUDUO_BASE_TIMESTAMP_H
#define MINI_MUDUO_BASE_TIMESTAMP_H

#include <stdint.h>
#include <string>
#include <time.h>

namespace mini_muduo {

// 微秒级时间戳类。
// 内部保存从 Unix Epoch 到当前时间的微秒数，0 表示无效时间。
class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    // 判断时间戳是否有效。
    bool valid() const;

    // 返回从 Unix Epoch 开始的微秒数。
    int64_t microSecondsSinceEpoch() const;

    // 返回从 Unix Epoch 开始的秒数。
    time_t secondsSinceEpoch() const;

    // 返回 "秒.微秒" 格式的字符串。
    std::string toString() const;

    // 返回 "yyyy-mm-dd hh:mm:ss" 格式的本地时间字符串。
    std::string toFormattedString() const;

    // 获取当前时间。
    static Timestamp now();

    // 返回一个无效时间戳。
    static Timestamp invalid();

private:
    int64_t microSecondsSinceEpoch_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_BASE_TIMESTAMP_H
