#include "base/Logger.h"
#include "net/EventLoop.h"
#include "net/EventLoopThreadPool.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

using namespace mini_muduo;

int main()
{
    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "testPool");
    pool.setThreadNum(3);
    pool.start();

    std::vector<EventLoop*> loops;
    for (int i = 0; i < 6; ++i) {
        EventLoop* loop = pool.getNextLoop();
        if (loop == nullptr) {
            LOG_ERROR("EventLoopThreadPool returned null loop");
            return 1;
        }
        loops.push_back(loop);
    }

    bool allBaseLoop = std::all_of(loops.begin(), loops.end(), [&](EventLoop* loop) {
        return loop == &baseLoop;
    });
    if (allBaseLoop) {
        LOG_ERROR("EventLoopThreadPool returned only baseLoop");
        return 1;
    }

    if (loops[0] != loops[3] || loops[1] != loops[4] || loops[2] != loops[5]) {
        LOG_ERROR("EventLoopThreadPool did not return loops in round-robin order");
        return 1;
    }

    for (EventLoop* loop : loops) {
        loop->queueInLoop([]() {
            LOG_INFO("task executed in pooled EventLoop");
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (EventLoop* loop : loops) {
        loop->quit();
    }

    LOG_INFO("test_event_loop_thread_pool success");
    return 0;
}
