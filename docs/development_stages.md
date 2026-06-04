# mini_muduo 开发阶段记录

## 阶段1：项目骨架

- 实现内容：建立 CMake 工程结构，划分 include、src、examples、tests、scripts 目录。
- 关键类：暂无核心类，重点是工程组织。
- 测试方式：通过 CMake 配置和基础示例编译验证。
- 面试价值：体现工程化意识和从零组织 C++ 项目的能力。

## 阶段2：Socket 封装

- 实现内容：封装 InetAddress、Socket、SocketsOps，提供地址、fd 和基础 socket 系统调用能力。
- 关键类：InetAddress、Socket、SocketsOps。
- 测试方式：运行 socket wrapper 相关测试。
- 面试价值：能说明 Linux Socket API、RAII 管理 fd 和非阻塞网络编程基础。

## 阶段3：Channel / Poller / EPollPoller

- 实现内容：实现事件分发对象 Channel、抽象 Poller 和 epoll 版本 EPollPoller。
- 关键类：Channel、Poller、EPollPoller。
- 测试方式：运行 epoll poller 测试，验证事件注册、更新和返回。
- 面试价值：能讲清楚 Reactor 中事件检测和事件回调的分层关系。

## 阶段4：EventLoop

- 实现内容：实现事件循环、活跃事件分发、eventfd 唤醒、runInLoop 和 queueInLoop。
- 关键类：EventLoop。
- 测试方式：运行 EventLoop 测试，验证循环、退出和跨线程任务投递。
- 面试价值：能说明 one loop per thread 的基础约束和跨线程唤醒机制。

## 阶段5：Acceptor

- 实现内容：封装监听 socket 和 accept 流程，在新连接到来时触发回调。
- 关键类：Acceptor。
- 测试方式：运行 Acceptor 测试，验证监听和新连接接收。
- 面试价值：能讲清楚服务端 listen fd 与 connfd 的职责区别。

## 阶段6：Buffer

- 实现内容：实现输入输出缓冲区，支持 readFd、append、retrieve 等操作。
- 关键类：Buffer。
- 测试方式：运行 Buffer 测试，验证读写索引和缓冲区扩容。
- 面试价值：能解释 TCP 粘包/半包背景，以及 readerIndex/writerIndex 的意义。

## 阶段7：TcpConnection

- 实现内容：封装单条 TCP 连接的读、写、关闭和回调处理。
- 关键类：TcpConnection。
- 测试方式：运行 TcpConnection 测试，验证连接建立、消息读写和关闭流程。
- 面试价值：能说明连接生命周期、shared_ptr 管理和异步发送处理。

## 阶段8：TcpServer

- 实现内容：整合 Acceptor 和 TcpConnection，管理连接表和服务器回调。
- 关键类：TcpServer、TcpConnection、Acceptor。
- 测试方式：运行 TcpServer 测试，验证新连接创建和关闭移除。
- 面试价值：能讲清楚从监听到连接对象创建的完整服务端流程。

## 阶段9：EventLoopThreadPool

- 实现内容：实现 EventLoopThread 和 EventLoopThreadPool，把连接分配到多个 EventLoop。
- 关键类：EventLoopThread、EventLoopThreadPool。
- 测试方式：运行线程池相关测试，验证线程启动和 loop 分配。
- 面试价值：能说明多 Reactor 模型和 one loop per thread 的并发设计。

## 阶段10：TimerQueue

- 实现内容：实现 Timer、TimerId、TimerQueue，基于 timerfd 管理定时任务。
- 关键类：Timer、TimerId、TimerQueue。
- 测试方式：运行 TimerQueue 测试，验证一次性和重复定时任务。
- 面试价值：能说明 timerfd 如何接入 epoll，以及为什么定时任务也能事件化。

## 阶段11：EchoServer 验收与压测

- 实现内容：提供 EchoServer、DiscardServer、BenchmarkClient 和集成测试。
- 关键类：TcpServer、TcpConnection、Buffer。
- 测试方式：运行 echo server 集成测试和 benchmark_client 压测。
- 面试价值：能证明网络库不是只停留在类封装，而是可以运行真实 TCP 服务。

## 阶段12：README 与文档收尾

- 实现内容：补充 README、架构文档、面试笔记、开发阶段记录和简历描述。
- 关键类：覆盖 Logger、Timestamp、Socket、Channel、Poller、EventLoop、Acceptor、Buffer、TcpConnection、TcpServer、EventLoopThreadPool、TimerQueue。
- 测试方式：运行 scripts/run_all_tests.sh，确认收尾后项目仍可编译和测试通过。
- 面试价值：把项目包装成可展示、可讲解、可复盘的 C++ 后端网络库项目。
