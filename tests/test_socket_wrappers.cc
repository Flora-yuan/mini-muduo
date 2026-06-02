#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/SocketsOps.h"

#include <cassert>
#include <type_traits>

int main()
{
    mini_muduo::InetAddress anyAddress(9000);
    assert(anyAddress.toIp() == "0.0.0.0");
    assert(anyAddress.toIpPort() == "0.0.0.0:9000");

    mini_muduo::InetAddress loopbackAddress(9001, true);
    assert(loopbackAddress.toIp() == "127.0.0.1");
    assert(loopbackAddress.toIpPort() == "127.0.0.1:9001");

    mini_muduo::InetAddress explicitAddress("127.0.0.1", 9002);
    assert(explicitAddress.toIp() == "127.0.0.1");
    assert(explicitAddress.toIpPort() == "127.0.0.1:9002");

    static_assert(!std::is_copy_constructible<mini_muduo::Socket>::value,
                  "Socket must own fd uniquely and must not be copy constructible");
    static_assert(!std::is_copy_assignable<mini_muduo::Socket>::value,
                  "Socket must own fd uniquely and must not be copy assignable");

    int fd = mini_muduo::sockets::createBlockingOrDie();
    mini_muduo::Socket socket(fd);
    assert(socket.fd() == fd);

    return 0;
}
