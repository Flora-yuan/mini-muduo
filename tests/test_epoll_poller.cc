#include "base/Logger.h"
#include "net/Channel.h"
#include "net/EPollPoller.h"
#include "net/Poller.h"
#include "net/SocketsOps.h"

#include <errno.h>
#include <unistd.h>

#include <cstring>
#include <string>

int main()
{
    LOG_INFO("test_epoll_poller start");

    int pipefd[2];
    if (::pipe(pipefd) < 0) {
        LOG_FATAL("pipe failed: " + std::string(std::strerror(errno)));
    }

    mini_muduo::sockets::setNonBlockAndCloseOnExec(pipefd[0]);

    mini_muduo::EPollPoller poller;
    mini_muduo::Channel readChannel(pipefd[0]);
    bool received = false;

    readChannel.setReadCallback([&]() {
        char buf[64] = {0};
        ssize_t n = ::read(pipefd[0], buf, sizeof(buf));
        if (n > 0) {
            received = true;
            LOG_INFO("received: " + std::string(buf, static_cast<size_t>(n)));
        }
    });

    readChannel.enableReading();
    poller.updateChannel(&readChannel);

    const char message[] = "hello epoll";
    ssize_t n = ::write(pipefd[1], message, sizeof(message) - 1);
    if (n < 0) {
        LOG_FATAL("write failed: " + std::string(std::strerror(errno)));
    }

    mini_muduo::Poller::ChannelList activeChannels;
    poller.poll(3000, &activeChannels);

    for (mini_muduo::Channel* channel : activeChannels) {
        channel->handleEvent();
    }

    if (received) {
        LOG_INFO("test_epoll_poller success");
    } else {
        LOG_ERROR("test_epoll_poller failed");
        poller.removeChannel(&readChannel);
        mini_muduo::sockets::close(pipefd[0]);
        mini_muduo::sockets::close(pipefd[1]);
        return 1;
    }

    poller.removeChannel(&readChannel);
    mini_muduo::sockets::close(pipefd[0]);
    mini_muduo::sockets::close(pipefd[1]);

    return 0;
}
