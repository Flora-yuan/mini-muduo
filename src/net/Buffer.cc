#include "net/Buffer.h"

#include <sys/uio.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>

namespace mini_muduo {

Buffer::Buffer(size_t initialSize)
    : buffer_(kCheapPrepend + initialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend)
{
}

size_t Buffer::readableBytes() const
{
    return writerIndex_ - readerIndex_;
}

size_t Buffer::writableBytes() const
{
    return buffer_.size() - writerIndex_;
}

size_t Buffer::prependableBytes() const
{
    return readerIndex_;
}

const char* Buffer::peek() const
{
    return begin() + readerIndex_;
}

void Buffer::retrieve(size_t len)
{
    if (len < readableBytes()) {
        readerIndex_ += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    const size_t readable = readableBytes();
    const size_t n = std::min(len, readable);
    std::string result(peek(), n);
    retrieve(n);
    return result;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string& str)
{
    append(str.data(), str.size());
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

char* Buffer::beginWrite()
{
    return begin() + writerIndex_;
}

const char* Buffer::beginWrite() const
{
    return begin() + writerIndex_;
}

void Buffer::hasWritten(size_t len)
{
    assert(len <= writableBytes());
    writerIndex_ += len;
}

char* Buffer::begin()
{
    return &*buffer_.begin();
}

const char* Buffer::begin() const
{
    return &*buffer_.begin();
}

void Buffer::makeSpace(size_t len)
{
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
        buffer_.resize(writerIndex_ + len);
    } else {
        const size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
    }
}

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = writable < sizeof extrabuf ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        if (savedErrno != NULL) {
            *savedErrno = errno;
        }
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += static_cast<size_t>(n);
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, static_cast<size_t>(n) - writable);
    }

    return n;
}

}  // namespace mini_muduo
