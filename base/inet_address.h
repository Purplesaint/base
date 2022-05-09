#pragma once

#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <net/if.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
void fromIpPort(std::string_view ip, uint16_t local_port,
                struct sockaddr_in6 *addr_out);

void fromIpPort(std::string_view ip, uint16_t local_port,
                struct sockaddr_in *addr_out);
std::string getReadableIp(const struct sockaddr_in &addr);
std::string getReadableIp(const struct sockaddr_in6 &addr);

static sockaddr *sockaddr_pointer_cast(sockaddr_in *ptr) {
  return reinterpret_cast<sockaddr *>(ptr);
}
static const sockaddr *sockaddr_pointer_cast(const sockaddr_in *ptr) {
  return reinterpret_cast<const sockaddr *>(ptr);
}

class InetAddress {
public:
  InetAddress() = default;
  InetAddress(const InetAddress &rhs) = default;
  InetAddress &operator=(const InetAddress &rhs) = default;

  InetAddress(InetAddress &&rhs) = default;
  InetAddress &operator=(InetAddress &&rhs) = default;

  InetAddress(std::string_view readable_ip, uint16_t local_port,
              int family = AF_INET);
  InetAddress(uint32_t ip, uint16_t port,bool network_byte_order = false) {
    addr_v4_.sin_family = AF_INET;
    addr_v4_.sin_addr.s_addr = ip;
    addr_v4_.sin_port = port;
    if( !network_byte_order ) {
      addr_v4_.sin_addr.s_addr =htonl(addr_v4_.sin_addr.s_addr);
      addr_v4_.sin_port =htons(addr_v4_.sin_port);
    }
  }
  InetAddress(const struct sockaddr_in &addr, bool network_byte_order = true)
      : addr_v4_(addr) {
    if (!network_byte_order) {
      addr_v4_.sin_addr.s_addr = htonl(addr_v4_.sin_addr.s_addr);
      addr_v4_.sin_port = htons(addr_v4_.sin_port);
    }
  }
  InetAddress(const struct sockaddr_in6 &addr, bool network_byte_order = true)
      : addr_v6_(addr) {
    // TODO unfinished
  }

  bool operator==(const InetAddress &rhs) {
    bool is_same_family = GetFamily() == rhs.GetFamily();
    if (!is_same_family) {
      return false;
    }

    if (IsIpv4()) {
      return addr_v4_.sin_addr.s_addr == rhs.addr_v4_.sin_addr.s_addr &&
             addr_v4_.sin_port == rhs.addr_v4_.sin_port;
    } else {
      return memcmp(&addr_v6_.sin6_addr, &rhs.addr_v6_.sin6_addr,
                    sizeof(addr_v6_.sin6_addr)) == 0 &&
             addr_v6_.sin6_port == rhs.addr_v6_.sin6_port;
    }
  }

  sa_family_t GetFamily() const {
    /* 此处可以把 sockaddr 看成sockaddr_in 和 sockaddr_in6 的基类 */
    /* 两个字类的前面的sa_family字段都处于相同的位置，所以直接取，以获得类型 */
    static_assert(offsetof(sockaddr_in, sin_family) ==
                      offsetof(sockaddr_in6, sin6_family),
                  "sockaddr family offset not same");
    return getSockAddr()->sa_family;
  }
  bool IsIpv4() const { return GetFamily() == AF_INET; }

  bool IsIpv6() const { return GetFamily() == AF_INET6; }

  uint32_t GetIpv4InLocalByteOrder() const {
    assert(IsIpv4());
    return ntohl(addr_v4_.sin_addr.s_addr);
  }
  uint32_t GetIpv4InNetworkByteOrder() const {
    assert(IsIpv4());
    return addr_v4_.sin_addr.s_addr;
  }

  uint16_t GetPortInNetworkByteOrder() const {
    static_assert(offsetof(sockaddr_in, sin_port) ==
                      offsetof(sockaddr_in6, sin6_port),
                  "sockaddr port offset not same");
    /* sockaddr_in 和 sockaddr_in6 的端口字段在内存中的偏移量是一样的 */
    return addr_v4_.sin_port;
  }

  uint16_t GetPortInLocalByteOrder() const {
    static_assert(offsetof(sockaddr_in, sin_port) ==
                      offsetof(sockaddr_in6, sin6_port),
                  "sockaddr port offset not same");
    /* sockaddr_in 和 sockaddr_in6 的端口字段在内存中的偏移量是一样的 */
    return ntohs(addr_v4_.sin_port);
  }
  const std::string &GetReadableIp() const {
    if (readable_ip_.empty()) {
      if (GetFamily() == AF_INET) {
        readable_ip_ = getReadableIp(addr_v4_);
      } else {
        readable_ip_ = getReadableIp(addr_v6_);
      }
    }
    return readable_ip_;
  }

  /* [ip:port] pair form */
  const std::string &GetAddressText() const {
    if (address_text_.empty()) {
      address_text_ = "[" + GetReadableIp() + ":" +
                      std::to_string(GetPortInLocalByteOrder()) + "]";
    }
    return address_text_;
  }
  socklen_t GetSockStrLen() const {
    return IsIpv4() ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
  }
  /* inet address was stored in byte order */
  const struct sockaddr *getSockAddr() const {
    return sockaddr_pointer_cast(&addr_v4_);
  }

  static std::string getReadableIpOf(std::string_view interface_name);
  static size_t InetAddressHash(const InetAddress &address);

private:
  /* was stored in network byte order */
  union {
    struct sockaddr_in addr_v4_;
    struct sockaddr_in6 addr_v6_;
  };

  mutable std::string readable_ip_;
  mutable std::string address_text_;
};