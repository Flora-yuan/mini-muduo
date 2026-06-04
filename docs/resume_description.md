# 简历项目描述

## 项目名称

基于 Reactor 模型的 C++ 高性能网络库

## 技术栈

C++11、Linux、Socket、epoll、Reactor、eventfd、timerfd、多线程、CMake

## 精简版（适合一页简历）

- 基于 Reactor 模型实现 C++ 网络库，封装 EventLoop、Channel、EPollPoller、Acceptor、TcpConnection、TcpServer 等核心模块。
- 基于 epoll 实现 IO 多路复用，使用非阻塞 Socket 与 Buffer 缓冲区处理 TCP 数据收发。
- 实现 one loop per thread 多线程模型，通过 EventLoopThreadPool 支持多连接并发处理。
- 使用 eventfd 实现跨线程唤醒，使用 timerfd 实现定时任务队列，提供 EchoServer、压测客户端和自动化测试脚本。

## 详细版（适合面试介绍）

我实现了一个基于 C++11 的简化版 muduo 网络库，项目核心是 Reactor 事件驱动模型。底层封装了 InetAddress、Socket 和 SocketsOps，使用非阻塞 Socket 和 epoll 实现 IO 多路复用；中间层通过 Channel 表示 fd 事件，通过 Poller/EPollPoller 负责事件检测，通过 EventLoop 统一完成事件等待、回调分发和跨线程任务执行。

在连接管理方面，我实现了 Acceptor、TcpConnection 和 TcpServer。Acceptor 负责监听和接收新连接，TcpServer 负责管理连接表和回调，TcpConnection 负责单条连接的读写、关闭和生命周期管理。发送数据时，如果一次 write 没有写完，会把剩余数据放入 outputBuffer，并在 fd 可写时继续发送。

项目还实现了 EventLoopThread 和 EventLoopThreadPool，支持 one loop per thread 多线程 Reactor 模型，让主 loop 负责接收连接，子 loop 负责连接上的 IO 事件处理。为了支持跨线程任务投递，我使用 eventfd 唤醒 EventLoop；为了支持定时任务，我使用 timerfd 实现 TimerQueue，并把定时事件统一接入 epoll。

最后，我提供了 EchoServer、DiscardServer、BenchmarkClient 和一键测试脚本，覆盖基础模块、连接管理、多线程事件循环、TimerQueue 和 EchoServer 集成测试。通过这个项目，我系统掌握了 Linux 网络编程、Reactor 模型、epoll、非阻塞 IO、TCP 缓冲区和 C++ 后端工程化设计。
