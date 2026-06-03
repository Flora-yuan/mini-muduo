#include "base/Logger.h"
#include "net/Buffer.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <string>

int main()
{
    LOG_INFO("test_buffer start");

    mini_muduo::Buffer buf;
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == 1024);
    assert(buf.prependableBytes() == 8);

    buf.append("hello", 5);
    assert(buf.readableBytes() == 5);

    std::string first = buf.retrieveAsString(2);
    assert(first == "he");
    assert(buf.readableBytes() == 3);

    std::string rest = buf.retrieveAllAsString();
    assert(rest == "llo");

    std::string big(2000, 'x');
    buf.append(big);
    assert(buf.readableBytes() == 2000);

    buf.retrieveAll();
    assert(buf.readableBytes() == 0);
    assert(buf.prependableBytes() == 8);

    int pipefd[2];
    assert(::pipe(pipefd) == 0);

    const std::string message = "hello buffer readFd";
    ssize_t written = ::write(pipefd[1], message.data(), message.size());
    assert(written == static_cast<ssize_t>(message.size()));
    ::close(pipefd[1]);

    int savedErrno = 0;
    ssize_t n = buf.readFd(pipefd[0], &savedErrno);
    assert(n == static_cast<ssize_t>(message.size()));
    assert(savedErrno == 0);
    assert(buf.retrieveAllAsString() == message);

    ::close(pipefd[0]);

    LOG_INFO("test_buffer success");
    return 0;
}
