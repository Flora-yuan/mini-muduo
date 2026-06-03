#include "base/Logger.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>

int main()
{
    LOG_INFO("test_event_loop start");

    int pipefd[2];
    if (::pipe(pipefd) < 0) {
        LOG_FATAL("pipe failed: " + std::string(std::strerror(errno)));
    }

    mini_muduo::sockets::setNonBlockAndCloseOnExec(pipefd[0]);
    mini_muduo::sockets::setNonBlockAndCloseOnExec(pipefd[1]);

    mini_muduo::EventLoop loop;
    mini_muduo::Channel readChannel(&loop, pipefd[0]);
    bool received = false;

    readChannel.setReadCallback([&]() {
        char buf[64] = {0};
        ssize_t n = ::read(pipefd[0], buf, sizeof(buf));
        if (n > 0) {
            received = true;
            LOG_INFO("received: " + std::string(buf, static_cast<size_t>(n)));
            loop.quit();
        }
    });

    readChannel.enableReading();

    std::thread writer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        const char message[] = "hello event loop";
        ssize_t n = ::write(pipefd[1], message, sizeof(message) - 1);
        if (n < 0) {
            LOG_FATAL("write failed: " + std::string(std::strerror(errno)));
        }
    });

    loop.loop();
    writer.join();

    readChannel.disableAll();
    loop.removeChannel(&readChannel);
    mini_muduo::sockets::close(pipefd[0]);
    mini_muduo::sockets::close(pipefd[1]);

    if (!received) {
        LOG_ERROR("test_event_loop failed");
        return 1;
    }

    LOG_INFO("test_event_loop success");
    return 0;
}
