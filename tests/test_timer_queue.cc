#include "base/Logger.h"
#include "net/EventLoop.h"
#include "net/TimerId.h"

#include <atomic>

int main()
{
    LOG_INFO("test_timer_queue start");

    mini_muduo::EventLoop loop;
    std::atomic<int> count(0);

    loop.runAfter(0.2, []() {
        LOG_INFO("runAfter executed");
    });

    mini_muduo::TimerId everyId;
    everyId = loop.runEvery(0.1, [&]() {
        ++count;
        LOG_INFO("runEvery tick");
        if (count >= 3) {
            loop.cancel(everyId);
            loop.quit();
        }
    });

    loop.loop();

    if (count >= 3) {
        LOG_INFO("test_timer_queue success");
        return 0;
    }

    LOG_ERROR("test_timer_queue failed");
    return 1;
}
