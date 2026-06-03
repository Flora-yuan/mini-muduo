#ifndef MINI_MUDUO_NET_BUFFER_H
#define MINI_MUDUO_NET_BUFFER_H

#include <sys/types.h>

#include <string>
#include <vector>

namespace mini_muduo {

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize);

    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;

    void retrieve(size_t len);
    void retrieveAll();

    std::string retrieveAllAsString();
    std::string retrieveAsString(size_t len);

    void append(const char* data, size_t len);
    void append(const std::string& str);

    void ensureWritableBytes(size_t len);

    char* beginWrite();
    const char* beginWrite() const;

    void hasWritten(size_t len);

    ssize_t readFd(int fd, int* savedErrno);

private:
    char* begin();
    const char* begin() const;

    void makeSpace(size_t len);

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_BUFFER_H
