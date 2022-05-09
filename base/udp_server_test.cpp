#include "udp_server.h"
#include "gtest/gtest.h"

TEST(TestRecv, AllCases) {
  InetAddress server_address("127.0.0.1", 9999);
  UdpServer server(server_address);
  EXPECT_TRUE(server_address == server.getLocalAddresss());
  std::string words = "abcde";

  bool is_called = false;

  UdpServer::OnMessageCallback callback = [&](InetAddress address,
                                              Buffer &buffer) {
    is_called = true;

    EXPECT_TRUE(address.GetReadableIp() == "127.0.0.1");
    EXPECT_TRUE(
        address.GetPortInLocalByteOrder() !=
        server_address
            .GetPortInLocalByteOrder()); // 不能相等，看是否有自连接的现象
    EXPECT_EQ(buffer.readableSize(), words.size());
    EXPECT_EQ(words, buffer.toStringView());

    return;
  };
  server.SetOnMessageCallback(std::move(callback));
  server.Start();

  Socket client_socket = Socket::CreateUdpSocket();

  client_socket.SendTo(server.getLocalAddresss(), words);

  sleep(1);

  EXPECT_TRUE(is_called);
}

TEST(TestSend, AllCases) {

  InetAddress server_address1("127.0.0.1", 9999),
      server_address2("127.0.0.1", 7765);

  UdpServer server1(server_address1), server2(server_address2);

  std::string message_to_s1 = "from s2", message_to_s2 = "from s1";

  bool s1_recved_message = false, s2_recved_message = false;
  server1.SetOnMessageCallback([&](InetAddress address, Buffer &buffer)
                               {
    s1_recved_message = true;
    EXPECT_TRUE(address == server_address2);

    EXPECT_EQ(buffer.toStringView(), message_to_s1); });

  server2.SetOnMessageCallback([&](InetAddress address, Buffer &buffer) {
    s2_recved_message = true;
    EXPECT_TRUE(address == server_address1);

    EXPECT_EQ(buffer.toStringView(), message_to_s2);

  });

  server1.Start();
  server2.Start();

  server1.SendTo(server_address2, std::make_shared<std::string>(message_to_s2));
  server2.SendTo(server_address1, std::make_shared<std::string>(message_to_s1));

  sleep(1);
  EXPECT_TRUE(s1_recved_message);
  EXPECT_TRUE(s2_recved_message);
}
