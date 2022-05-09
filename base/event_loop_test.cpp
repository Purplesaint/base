#include "event_loop.h"
#include "time_base.h"
#include "gtest/gtest.h"

class TestEventLoop : public testing::Test {
  void SetUp() override {
    thread = std::thread([this]() { loop.loop(); });
  }
  void TearDown() override {
    loop.quit();
    thread.join();
  }

public:
  EventLoop loop;
  std::thread thread;
};
TEST_F(TestEventLoop, TestRunAfter) {
  {
    bool changed = false;
    loop.runAfter(TimeDuration(0), [&changed]() { changed = true; });

    usleep(1000 * 1000);
    EXPECT_TRUE(changed) << "0 ms";
  }

  {
    bool changed = false;
    loop.runAfter(TimeDuration(1), [&changed]() { changed = true; });

    usleep(100 * 1000);
    EXPECT_TRUE(changed) << "1 ms";
  }

  {
    bool changed = false;
    loop.runAfter(TimeDuration(1000), [&changed]() { changed = true; });
    usleep(500 * 1000);
    EXPECT_FALSE(changed) << "1000ms ";
    sleep(1);

    EXPECT_TRUE(changed);
  }
}

TEST_F(TestEventLoop, TestRunEvery) {
  {
    int cnt = 0;
    auto id = loop.runEvery(TimeDuration(1000), [&cnt]() { cnt++; });
    sleep(1);
    usleep(1 * 1000);
    EXPECT_EQ(1, cnt);

    sleep(1);
    usleep(1 * 1000);
    EXPECT_EQ(2, cnt);

    EXPECT_TRUE(loop.cancel(id));
    EXPECT_FALSE(loop.cancel(id));

    sleep(1);
    usleep(1 * 1000);
    EXPECT_EQ(2, cnt);
  }

  {
    int cnt = 1;
    auto now_time = now();
    auto interval = TimeDuration(50);
    auto id = loop.runEvery(interval, [&]() {
      auto eclapse = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now() - now_time)
                         .count();

      ASSERT_GE(eclapse, (interval.count() * cnt -
                          10)); // min 10 to maintain eclapse is always bigger

      auto remain = eclapse - (interval.count() * cnt - 10);
      EXPECT_LE(remain, 15); // 误差尽量小
      ++cnt;
    });

    auto wait_times = 5;
    usleep(wait_times * interval.count() * 1000);

    EXPECT_TRUE(loop.cancel(id));
    EXPECT_FALSE(loop.cancel(id));

    sleep(1);
    usleep(1 * 1000);
    EXPECT_LE(abs(cnt - wait_times), 1);
  }
}