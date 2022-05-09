#include "inet_address.h"
#include "gtest/gtest.h"

TEST(TestFromIpPort, AllCases) {
  {
    sockaddr_in addr = {0};
    fromIpPort("127.0.0.2", 23, &addr);
    InetAddress inet_addr(addr);

    EXPECT_EQ(inet_addr.GetReadableIp(),"127.0.0.2");
    EXPECT_EQ(inet_addr.GetPortInLocalByteOrder(), 23);
    EXPECT_EQ(inet_addr.GetPortInNetworkByteOrder(), htons(23));
  }
}
TEST(TestInetAddress, AllCases) {
  {
    InetAddress inet_addr("127.0.0.1", 2, AF_INET);

    EXPECT_EQ(inet_addr.GetReadableIp(), "127.0.0.1");
    EXPECT_EQ(inet_addr.GetPortInLocalByteOrder(), 2);
    EXPECT_EQ(inet_addr.GetPortInNetworkByteOrder(), htons(2));
    EXPECT_TRUE(inet_addr.IsIpv4());
  }
}