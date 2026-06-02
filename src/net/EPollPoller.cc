#include "net/EPollPoller.h"

#include "base/Logger.h"
#include "net/Channel.h"

#include <errno.h>
#include <unistd.h>

#include <cstring>
#include <string>

namespace mini_muduo {

namespace {

// Channel 在 Poller 中的状态约定。
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

std::string errnoMessage(const std::string& action)
{
    return action + " failed: " + std::strerror(errno);
}

const char* operationToString(int operation)
{
    switch (operation) {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        return "UNKNOWN";
    }
}

}  // namespace

EPollPoller::EPollPoller()
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        LOG_FATAL(errnoMessage("epoll_create1"));
    }
}

EPollPoller::~EPollPoller()
{
    if (::close(epollfd_) < 0) {
        LOG_ERROR(errnoMessage("close epollfd"));
    }
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    // epoll_wait 会把就绪事件写入 events_ 数组。
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_DEBUG("epoll_wait timeout");
    } else if (errno != EINTR) {
        LOG_ERROR(errnoMessage("epoll_wait"));
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    const int fd = channel->fd();

    if (index == kNew || index == kDeleted) {
        // 新 Channel 第一次加入时，需要登记到 channels_ 映射中。
        if (index == kNew) {
            channels_[fd] = channel;
        }
        update(EPOLL_CTL_ADD, channel);
        channel->set_index(kAdded);
    } else if (index == kAdded) {
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel)
{
    const int fd = channel->fd();
    channels_.erase(fd);

    if (channel->index() == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, channel->fd(), &event) < 0) {
        std::string message = "epoll_ctl " + std::string(operationToString(operation)) +
                              " fd " + std::to_string(channel->fd()) + " " +
                              std::strerror(errno);
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR(message);
        } else {
            LOG_FATAL(message);
        }
    }
}

}  // namespace mini_muduo
