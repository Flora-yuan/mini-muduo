#ifndef MINI_MUDUO_NET_CHANNEL_H
#define MINI_MUDUO_NET_CHANNEL_H

#include "base/nocopyable.h"

#include <stdint.h>
#include <functional>

namespace mini_muduo {

class EventLoop;

// Channel 封装一个 fd 上关注的事件和事件发生后的回调。
// 绑定 EventLoop 后，事件关注变化会自动同步到 Poller。
class Channel : public nocopyable {
public:
    using EventCallback = std::function<void()>;

    explicit Channel(int fd);
    Channel(EventLoop* loop, int fd);
    ~Channel();

    int fd() const;
    uint32_t events() const;
    uint32_t revents() const;
    void set_revents(uint32_t revt);

    bool isNoneEvent() const;

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

    bool isWriting() const;
    bool isReading() const;

    int index() const;
    void set_index(int index);

    void setReadCallback(EventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);

    void handleEvent();
    void remove();

private:
    void update();
    void handleEventWithGuard();

private:
    EventLoop* loop_;
    const int fd_;
    uint32_t events_;
    uint32_t revents_;
    int index_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_CHANNEL_H
