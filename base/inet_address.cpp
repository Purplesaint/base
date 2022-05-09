#include "inet_address.h"
#include <unistd.h>

void fromIpPort(std::string_view ip, uint16_t port,
                struct sockaddr_in6 *addr_out) {
  if (!addr_out) {
    throw std::invalid_argument("invalid addr_out");
  }

  addr_out->sin6_port = htons(port); // ：认为网络字节序
  addr_out->sin6_family = AF_INET6;

  if (inet_pton(AF_INET6, ip.data(), &addr_out->sin6_addr) != 1) {
    throw std::invalid_argument("invalid ip");
  }
  return;
}

void fromIpPort(std::string_view ip, uint16_t port,
                struct sockaddr_in *addr_out) {
  if (!addr_out) {
    throw std::invalid_argument("invalid addr_out");
  }

  addr_out->sin_port = htons(port); // ：认为网络字节序
  addr_out->sin_family = AF_INET;
  if (inet_pton(AF_INET, ip.data(), &addr_out->sin_addr) != 1) {
    throw std::invalid_argument("invalid ip");
  }
  return;
}

/* 转换ipv4和ipv6地址，不对外暴露 */
static std::string getReadableIp(const void *ip_buf_ptr,
                                 uint64_t ip_str_length) {
  if (!ip_buf_ptr) {
    throw std::invalid_argument("invalid addr: empty arg");
  }

  std::string buf(ip_str_length, '\0');
  if (!inet_ntop(AF_INET, ip_buf_ptr, &buf[0], ip_str_length)) {
    throw errno;
  }

  auto pos = buf.find_first_of('\0');
  /* find the real size of buf */
  buf.resize(pos);

  return std::move(buf);
}

std::string getReadableIp(const struct sockaddr_in &addr) {
  return getReadableIp(&addr.sin_addr, INET_ADDRSTRLEN);
}

std::string getReadableIp(const struct sockaddr_in6 &addr) {
  return getReadableIp(&addr.sin6_addr, INET6_ADDRSTRLEN);
}

InetAddress::InetAddress(std::string_view readable_ip, uint16_t port,
                         int family) {
  if (family == AF_INET) {
    fromIpPort(readable_ip, port, &addr_v4_);
  } else {
    fromIpPort(readable_ip, port, &addr_v6_);
  }

  /* 直接保存使用 */
  readable_ip_ = readable_ip;
}

std::string InetAddress::getReadableIpOf(std::string_view interface_name) {
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;

  bzero(ifr.ifr_name, IFNAMSIZ);
  strncpy(ifr.ifr_name, interface_name.data(), interface_name.size());

  auto fd = socket(AF_INET, SOCK_DGRAM, 0);
  auto ret = ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);

  if (ret < 0) {
    printf("errno %d\n", errno);
    throw errno;
  }

  sockaddr_in &addr = reinterpret_cast<sockaddr_in &>(ifr.ifr_addr);
  return getReadableIp(addr);
}

static size_t InetAddressHash(const InetAddress &address) {
  return std::hash<std::string>()(address.GetAddressText());
}
