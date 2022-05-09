#include "socket.h"
#include "work_thread.h"
#include "gtest/gtest.h"

class TestWorkThread : public testing::Test {
public:
  TestWorkThread()
      : worker_("test_worker"), socket_(Socket::CreateUdpSocket()) {}
  void SetUp() override {

    socket_.bind(binded_address_);

    p_object = std::make_shared<IoObject>(socket_.getFd());
  }
  void TearDown() override {}
  WorkThread worker_;
  std::shared_ptr<IoObject> p_object;
  Socket socket_;
  InetAddress binded_address_{"127.0.0.1", 7755};
};
TEST_F(TestWorkThread, AllCases) {
  bool read_event_happened = false;
  p_object->EnableEvent(IoEventType::kReadEvent);
  p_object->sig_new_read_event.connect(
      [&read_event_happened]() { read_event_happened = true; });
  worker_.start();
  worker_.getLoop()->addIoObject(p_object);

  send_to(socket_.getFd(), binded_address_, "abc");

  sleep(1);
  EXPECT_TRUE(read_event_happened);
}
