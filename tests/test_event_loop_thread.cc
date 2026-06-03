#include "base/Logger.h"
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace mini_muduo;

int main()
{
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();

    if (loop == nullptr) {
        LOG_ERROR("EventLoopThread returned null loop");
        return 1;
    }

    std::atomic<bool> executed(false);
    loop->queueInLoop([&]() {
        LOG_INFO("task executed in EventLoopThread");
        executed = true;
        loop->quit();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (!executed.load()) {
        LOG_ERROR("queued task was not executed");
        return 1;
    }

    LOG_INFO("test_event_loop_thread success");
    return 0;
}
