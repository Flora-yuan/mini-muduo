#ifndef MINI_MUDUO_NET_EPOLLPOLLER_H
#define MINI_MUDUO_NET_EPOLLPOLLER_H

#include "net/Poller.h"

#include <sys/epoll.h>

namespace mini_muduo {

// EPollPoller 使用 Linux epoll 实现 Poller 接口。
class EPollPoller : public Poller {
public:
    EPollPoller();
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;
    using EventList = std::vector<struct epoll_event>;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

private:
    int epollfd_;
    EventList events_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_EPOLLPOLLER_H
