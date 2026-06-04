# mini_muduo 面试复习笔记

## 1. 什么是 Reactor 模型？

Reactor 是一种事件驱动模型：程序先在事件循环中等待 IO 事件，事件到来后再分发给对应的处理器。本项目中 EventLoop 等待事件，Channel 保存事件和回调，TcpConnection 处理连接读写。

## 2. Channel 的作用是什么？

Channel 是 fd 的事件代理，保存 fd 关注的事件、实际发生的事件和对应回调。它让 EventLoop 不需要关心具体业务对象，只负责事件分发。

## 3. EventLoop 的作用是什么？

EventLoop 是事件循环核心，负责调用 Poller 等待事件、执行 Channel 回调、处理跨线程任务和定时任务。一个 EventLoop 通常绑定一个线程。

## 4. Poller 和 EPollPoller 的区别？

Poller 是抽象接口，定义统一的事件检测方法；EPollPoller 是基于 Linux epoll 的具体实现。这样可以让 EventLoop 依赖抽象，降低和 epoll 细节的耦合。

## 5. 为什么使用 epoll？

epoll 适合 Linux 下大量 fd 的事件监听。相比 select，它没有固定 fd 数量限制，并且可以通过内核维护事件集合，活跃连接较少时效率更高。

## 6. LT 和 ET 有什么区别？本项目使用哪个？

LT 是水平触发，只要 fd 仍然可读或可写，就会持续通知；ET 是边沿触发，只在状态变化时通知一次。本项目使用 LT，逻辑更简单，也更适合教学和稳定验证。

## 7. 为什么需要非阻塞 IO？

事件循环不能被某个连接的 read 或 write 卡住。非阻塞 IO 可以保证一次读写不能完成时立即返回，让 EventLoop 继续处理其他连接。

## 8. 为什么需要 eventfd？

当其他线程向某个 EventLoop 投递任务时，该 EventLoop 可能正阻塞在 epoll_wait。eventfd 可以让其他线程写入一个事件，唤醒 EventLoop 及时处理任务队列。

## 9. runInLoop 和 queueInLoop 区别？

runInLoop 如果当前线程就是 EventLoop 所在线程，会直接执行任务；否则会入队并唤醒 EventLoop。queueInLoop 总是把任务放入队列，适合需要统一异步执行的场景。

## 10. TcpConnection 如何管理生命周期？

TcpConnection 创建后由 TcpServer 保存到连接表中，建立连接时启用读事件，关闭时触发 closeCallback，由 TcpServer 移除连接并清理 Channel。

## 11. 为什么 TcpConnection 使用 shared_ptr？

连接对象可能在多个回调和异步任务中被使用。shared_ptr 可以保证回调执行期间对象仍然存活，避免连接关闭后对象提前释放导致悬空引用。

## 12. Buffer 为什么需要 readerIndex 和 writerIndex？

readerIndex 表示可读数据起点，writerIndex 表示可写位置。两者配合可以区分已读、可读、可写区域，避免频繁移动内存，并支持 TCP 粘包/半包处理。

## 13. send 没有一次写完怎么办？

先尽量直接 write，没写完的数据放入 outputBuffer，然后让 Channel 关注可写事件。等 socket 再次可写时，handleWrite 继续发送剩余数据。

## 14. Acceptor 的作用是什么？

Acceptor 管理监听 socket。当有新连接到来时，它负责 accept，得到 connfd 后通过回调交给 TcpServer 创建 TcpConnection。

## 15. TcpServer 如何管理连接？

TcpServer 持有连接 map。新连接建立时创建 TcpConnection 并保存，连接关闭时通过回调从 map 中移除，实现连接生命周期管理。

## 16. one loop per thread 是什么？

一个线程运行一个 EventLoop，每个连接归属于某个固定 EventLoop。这样可以减少锁竞争，让连接的 IO 事件在同一线程内顺序处理。

## 17. TimerQueue 为什么使用 timerfd？

timerfd 可以把定时器变成一个 fd，超时时 fd 可读。这样 TimerQueue 可以接入 epoll，由 EventLoop 统一处理 IO 事件和定时任务。

## 18. 这个项目和普通 WebServer 有什么区别？

普通 WebServer 更偏应用层，重点是 HTTP 解析、路由和业务处理；mini_muduo 是网络库，重点是底层 TCP 连接管理、事件循环、IO 多路复用和多线程 Reactor。

## 19. 这个项目有哪些可以优化的地方？

可以加入更完整的日志系统、异步日志、连接超时管理、更多协议示例、更完善的错误处理、性能统计、单元测试覆盖率和更接近 muduo 的线程安全细节。

## 20. 如何把这个项目和 AI Agent 后端结合？

可以把 mini_muduo 作为高并发 TCP 服务基础，承接 Agent 请求、任务状态推送或长连接通信。上层可以增加 HTTP/WebSocket 协议解析，再对接模型推理、任务队列和工具调用模块。
