#include "event_loop.h"
#include <sys/timerfd.h>
#include <unistd.h>

EventLoop::TimerId EventLoop::addTimer(TimeDuration interval, Callback cb,
                                       bool one_shot) {

  auto timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  assert(timer_fd > 0);

  auto interval_ms = interval.count();

  auto interval_s = std::chrono::duration_cast<std::chrono::seconds>(interval).count();

  auto interval_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(interval - std::chrono::seconds(interval_s)).count();

  if (interval_ms == 0) { // means execute right away
    interval_ns = 1; // set to a non-zero value so it can be alarmed by timerfd. if set to zero ,it won't be executed
  }

  itimerspec new_time{
      .it_interval = {0},
      .it_value = {.tv_sec = interval_s, .tv_nsec = interval_ns}};


  if ( !one_shot ) {
    new_time.it_interval.tv_nsec = interval_ns;
    new_time.it_interval.tv_sec = interval_s;
  }

  auto result = timerfd_settime(timer_fd, 0, &new_time, nullptr); // 不需要close
  assert(result == 0);

  auto p_io_object = std::make_shared<IoObject>(timer_fd);

  p_io_object->EnableEvent(IoEventType::kReadEvent);

  p_io_object->sig_new_read_event.connect(
      [one_shot, timer_fd, cb = std::move(cb), this]() {
        uint64_t timeout_count = 0;
        auto n_bytes = ::read(timer_fd, &timeout_count, sizeof(timeout_count));
        assert(n_bytes == sizeof(timeout_count));
        /* call callback */
        if (cb) {
          cb();
        }

        /* if one shot , after call callback once , remove it */
        if (one_shot) {
          removeTimer(timer_fd);
        }
      });

  addIoObject(p_io_object);

  {
    std::lock_guard<std::mutex> guard(mutex_);
    auto [iter, emplace_successfully] =
        timer_fd_to_info_map_.emplace(timer_fd, p_io_object);
    assert(emplace_successfully);
  }

  return timer_fd;
}
bool EventLoop::removeTimer(TimerId id) {
  bool is_removed = false;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    if(timer_fd_to_info_map_.find(id) != timer_fd_to_info_map_.end()) {
      deleteIoObject(timer_fd_to_info_map_.at(id));
    }

    is_removed = timer_fd_to_info_map_.erase(id);

  }
  return is_removed;
}
