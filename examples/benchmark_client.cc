#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>

namespace {

bool writeAll(int fd, const char* data, size_t len)
{
    size_t written = 0;
    while (written < len) {
        ssize_t n = ::write(fd, data + written, len - written);
        if (n <= 0) {
            return false;
        }
        written += static_cast<size_t>(n);
    }
    return true;
}

bool readExact(int fd, char* data, size_t len)
{
    size_t readBytes = 0;
    while (readBytes < len) {
        ssize_t n = ::read(fd, data + readBytes, len - readBytes);
        if (n <= 0) {
            return false;
        }
        readBytes += static_cast<size_t>(n);
    }
    return true;
}

}  // namespace

int main(int argc, char* argv[])
{
    std::string ip = "127.0.0.1";
    int port = 9001;
    int requestCount = 10000;

    if (argc >= 2) {
        ip = argv[1];
    }
    if (argc >= 3) {
        port = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        requestCount = std::stoi(argv[3]);
    }

    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        std::cerr << "socket failed\n";
        return 1;
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "invalid ip: " << ip << "\n";
        ::close(fd);
        return 1;
    }

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        std::cerr << "connect failed: " << std::strerror(errno) << "\n";
        ::close(fd);
        return 1;
    }

    const std::string message = "hello mini_muduo benchmark";
    std::string response(message.size(), 0);
    int success = 0;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < requestCount; ++i) {
        if (!writeAll(fd, message.data(), message.size())) {
            break;
        }
        if (!readExact(fd, &response[0], response.size())) {
            break;
        }
        if (response == message) {
            ++success;
        }
    }
    auto finish = std::chrono::steady_clock::now();

    ::close(fd);

    std::chrono::duration<double> elapsed = finish - start;
    double seconds = elapsed.count();
    double qps = seconds > 0.0 ? success / seconds : 0.0;

    std::cout << "requests: " << requestCount << "\n";
    std::cout << "success: " << success << "\n";
    std::cout << "time: " << seconds << "s\n";
    std::cout << "qps: " << static_cast<int>(qps) << "\n";

    return success == requestCount ? 0 : 1;
}
