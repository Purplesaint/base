#include "inet_address.h"
#include <optional>
#include <sys/socket.h>
#include <sys/types.h>
#include <tuple>
#include <vector>
#include <cerrno>

static inline ssize_t send_to(int fd, InetAddress address_in_local_byte,
                std::string_view data) {
  assert(fd > 0);
  return ::sendto(fd, data.data(), data.length(), 0,
                  address_in_local_byte.getSockAddr(),
                  address_in_local_byte.GetSockStrLen());
}

static inline std::tuple<error_t,InetAddress, std::vector<uint8_t>>
recv_from(int fd) {
  auto buffer = std::vector<uint8_t>(2048, 0);
  InetAddress address;

  sockaddr_in c_addr = {0};
  socklen_t length = sizeof(c_addr);

  error_t error = 0;
  auto result = ::recvfrom(fd, &(buffer)[0], buffer.size(), 0,
                           sockaddr_pointer_cast(&c_addr), &length);
  if (result > 0) {
    buffer.resize(result);
    address = InetAddress(c_addr);
  }
  else
  {
    buffer.clear();
    error = errno;
  }

  return {error, std::move(address), std::move(buffer)};
}