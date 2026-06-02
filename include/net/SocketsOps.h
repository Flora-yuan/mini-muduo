#ifndef MINI_MUDUO_NET_SOCKETSOPS_H
#define MINI_MUDUO_NET_SOCKETSOPS_H

#include <cstddef>
#include <sys/types.h>

struct sockaddr;
struct sockaddr_in;

namespace mini_muduo {
namespace sockets {

int createNonblockingOrDie();
int createBlockingOrDie();

void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);

int accept(int sockfd, struct sockaddr_in* addr);

ssize_t read(int sockfd, void* buf, size_t count);
ssize_t write(int sockfd, const void* buf, size_t count);

void close(int sockfd);

void setNonBlockAndCloseOnExec(int sockfd);

}  // namespace sockets
}  // namespace mini_muduo

#endif  // MINI_MUDUO_NET_SOCKETSOPS_H
