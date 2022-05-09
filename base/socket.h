#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <exception>
#include <memory>

#include "buffer.h"
#include "inet_address.h"
#include "socket_helper.h"

class Socket {
public:
  static Socket CreateUdpSocket();
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;
  Socket(Socket &&) = default;
  Socket &operator=(Socket &&) = delete;
  ~Socket();
  int getFd() { return fd_; }

  void bind(const InetAddress& inet_address);
  void listen(int backlog = SOMAXCONN);
  std::pair<int, InetAddress> accept();
  void connect(const InetAddress& inet_address);

  std::pair<error_t, InetAddress> recv(Buffer &buffer) {
    sockaddr_in peer_sock_addr = {0};
    socklen_t sock_length = INET_ADDRSTRLEN;

    auto [error_code, peer_address, data] = recv();
    if (error_code == 0) {
      buffer.write(&data[0], data.size());
    }
    return {error_code, peer_address};
  }
  std::tuple<error_t, InetAddress, std::vector<uint8_t>> recv() {

    auto [error_code, peer_address, data] = recv_from(fd_);
    return {error_code, peer_address, data};
  }

  ssize_t SendTo(const InetAddress &peer_address,
                 const std::string_view &buffer) {
    auto remaining_bytes = buffer.size();
    std::size_t offset = 0;
    while (remaining_bytes > 0) {
      auto ret =
          send_to(fd_, peer_address,
                  std::string_view{buffer.data() + offset, remaining_bytes});
      if (ret < 0)
      {
        return ret;
      }
      remaining_bytes -= ret;
      offset = buffer.size() - remaining_bytes;
    }
    return offset;
  }

  void setReuseAddr(bool on) {
    /* 手册中写要为int值指代 */
    int opt_val = on;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) <
        0) {
      throw errno;
    }
  }
  void setReusePort(bool on) {
    /* 手册中写要为int值指代 */
    int opt_val = on;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val)) <
        0) {
      throw errno;
    }
  }

  InetAddress getAddress() const {
    if (fd_ < 0) {
      throw std::invalid_argument("invalid fd");
    }
    sockaddr_in addr{0};
    socklen_t length = sizeof(addr);
    int ret = getsockname(fd_, sockaddr_pointer_cast(&addr), &length);
    if (ret < 0) {
      throw errno;
    }

    return InetAddress(addr);
  }
  InetAddress getPeerAddress() const {
    if (fd_ < 0) {
      throw std::invalid_argument("invalid fd");
    }
    sockaddr_in addr{0};
    socklen_t length = sizeof(addr);
    int ret = getpeername(fd_, sockaddr_pointer_cast(&addr), &length);
    if (ret < 0) {
      throw errno;
    }

    return InetAddress(addr);
  }

  bool IsSelfConnected() const { return getAddress() == getPeerAddress(); }

private:
  explicit Socket(int fd) : fd_(fd) {}
  int fd_ = -1;
};