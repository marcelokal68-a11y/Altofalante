#include "altofalante/net.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <cstring>
#include <ctime>

namespace af::net {

double monotonicNow() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

bool Addr::operator==(const Addr& o) const {
    return sa.sin_addr.s_addr == o.sa.sin_addr.s_addr &&
           sa.sin_port == o.sa.sin_port;
}

UdpSocket::~UdpSocket() { close(); }

bool UdpSocket::open() {
    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    return fd_ >= 0;
}

bool UdpSocket::bindPort(uint16_t port) {
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons(port);
    return ::bind(fd_, (sockaddr*)&a, sizeof(a)) == 0;
}

void UdpSocket::close() {
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
}

bool UdpSocket::sendTo(const void* data, size_t len, const Addr& to) {
    return ::sendto(fd_, data, len, 0, (const sockaddr*)&to.sa, sizeof(to.sa)) ==
           (ssize_t)len;
}

int UdpSocket::recv(void* buf, size_t len, Addr& from, int timeoutMs) {
    fd_set rs;
    FD_ZERO(&rs);
    FD_SET(fd_, &rs);
    timeval tv{timeoutMs / 1000, (timeoutMs % 1000) * 1000};
    int r = ::select(fd_ + 1, &rs, nullptr, nullptr, &tv);
    if (r <= 0) return 0; // timeout ou erro tratado como timeout
    socklen_t sl = sizeof(from.sa);
    ssize_t n = ::recvfrom(fd_, buf, len, 0, (sockaddr*)&from.sa, &sl);
    return (int)n;
}

uint16_t UdpSocket::localPort() {
    sockaddr_in a{};
    socklen_t sl = sizeof(a);
    if (::getsockname(fd_, (sockaddr*)&a, &sl) != 0) return 0;
    return ntohs(a.sin_port);
}

bool UdpSocket::joinMulticast(const std::string& group, uint16_t port,
                             const std::string& iface) {
    int yes = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#ifdef SO_REUSEPORT
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif
    if (!bindPort(port)) return false;
    ip_mreq mreq{};
    inet_pton(AF_INET, group.c_str(), &mreq.imr_multiaddr);
    inet_pton(AF_INET, iface.c_str(), &mreq.imr_interface);
    return ::setsockopt(fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == 0;
}

bool UdpSocket::setMulticastIface(const std::string& iface) {
    in_addr a{};
    inet_pton(AF_INET, iface.c_str(), &a);
    return ::setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_IF, &a, sizeof(a)) == 0;
}

bool UdpSocket::enableMulticastLoop() {
    unsigned char on = 1;
    return ::setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &on, sizeof(on)) == 0;
}

Addr UdpSocket::makeAddr(const std::string& host, uint16_t port) {
    Addr a;
    a.sa.sin_family = AF_INET;
    a.sa.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &a.sa.sin_addr);
    return a;
}

} // namespace af::net
