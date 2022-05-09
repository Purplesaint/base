#include "gtest/gtest.h"

#include "socket.h"

TEST(TestSocket,TestNonBlock)
{
  Socket server_fd = Socket::CreateUdpSocket();
  auto [error_code,ignored1,data] = server_fd.recv(); //no block
  EXPECT_EQ(error_code,EAGAIN);
  EXPECT_TRUE(data.empty());
}

TEST(TestSocket, AllCases) {
  {
    Socket server_fd = Socket::CreateUdpSocket();
    InetAddress server_address("127.0.0.1",9999);
    EXPECT_GE(server_fd.getFd(), 0);
    server_fd.bind(server_address);

    std::string content("abc");

    Socket client_socket = Socket::CreateUdpSocket();
    EXPECT_GE(server_fd.getFd(), 0);
    EXPECT_GE(client_socket.SendTo(server_address,"abc"),0);
    client_socket.SendTo(server_address,content);

    auto[error_code,peer_address,data] = server_fd.recv();
    EXPECT_EQ(error_code, 0);

    EXPECT_EQ(peer_address.GetReadableIp(), "127.0.0.1");
    EXPECT_EQ(strcmp("abc", reinterpret_cast<const char *>(&data[0])) , 0);
  }
}