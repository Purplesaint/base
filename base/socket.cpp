#include "socket.h"
Socket Socket::CreateUdpSocket() {
  int fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  if (fd < 0) {
    throw errno;
  }
  return Socket(fd);
}

void Socket::bind(const InetAddress& inet_address) {
  if (::bind(fd_, inet_address.getSockAddr(), inet_address.GetSockStrLen()) <
      0)
  {
    throw errno;
  }
}

void Socket::listen(int backlog) {
  if (::listen(fd_, backlog) < 0) {
    throw errno;
  }
}
void Socket::connect(const InetAddress& inet_address) {
  if (::connect(fd_, inet_address.getSockAddr(), inet_address.GetSockStrLen()) <
      0)
  {
    throw errno;
  }
}

std::pair<int, InetAddress> Socket::accept() {
  sockaddr_in sock_addr = {0};
  socklen_t addr_len = INET_ADDRSTRLEN;
  int conn_fd =
      ::accept4(fd_, reinterpret_cast<sockaddr *>(&sock_addr), &addr_len, SOCK_NONBLOCK);
  auto peer_addr = InetAddress(sock_addr);
  return std::make_pair(conn_fd, std::move(peer_addr));
}

Socket::~Socket() {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}