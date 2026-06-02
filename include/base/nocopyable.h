#ifndef MINI_MUDUO_BASE_NOCOPYABLE_H
#define MINI_MUDUO_BASE_NOCOPYABLE_H

namespace mini_muduo {

// 不可拷贝基类。
// 继承该类的对象不能被拷贝构造，也不能被赋值，
// 常用于表示文件描述符、线程、连接等唯一资源的拥有者。
class nocopyable {
protected:
    // 只允许派生类构造和析构，避免直接创建 nocopyable 对象。
    nocopyable() = default;
    ~nocopyable() = default;

public:
    // 禁止拷贝构造。
    nocopyable(const nocopyable&) = delete;

    // 禁止赋值操作。
    nocopyable& operator=(const nocopyable&) = delete;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_BASE_NOCOPYABLE_H
