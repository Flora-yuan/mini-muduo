#ifndef MINI_MUDUO_NET_POLLER_H
#define MINI_MUDUO_NET_POLLER_H

#include "base/Timestamp.h"
#include "base/nocopyable.h"

#include <map>
#include <vector>

namespace mini_muduo {

class Channel;

// Poller 是事件监听器的抽象基类。
// 它负责管理 fd 到 Channel 的映射，具体等待事件的方式由派生类实现。
class Poller : public nocopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller();
    virtual ~Poller();

    // 等待事件发生，并把活跃 Channel 填入 activeChannels。
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    // 新增或修改 Channel 关注的事件。
    virtual void updateChannel(Channel* channel) = 0;

    // 从 Poller 中移除 Channel。
    virtual void removeChannel(Channel* channel) = 0;

    bool hasChannel(Channel* channel) const;

protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_POLLER_H
