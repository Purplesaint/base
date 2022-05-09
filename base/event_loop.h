#pragma once

#include "io/epoller.h"
#include <atomic>
#include <thread>

#include "time_base.h"

class EventLoop {
public:
  using Callback = std::function<void()>;
  using TimerId = int; // NOTE: 即fd,要着重验证是否重复的问题
  EventLoop() = default;
  ~EventLoop() = default;

  void loop() {
    quit_ = false;

    while (!quit_) {
      static constexpr int kWaitTimeMs = 1000;
      auto active_object_list =
          epoll_.wait(kWaitTimeMs); // has handle its object event
      for (auto &object_pair : active_object_list) {
        auto [object_fd, p_object] = object_pair;

        p_object->TriggerEvent();
      }
    }
  }
  void quit() { quit_ = true; }
  TimerId runEvery(TimeDuration interval, Callback callback) {
    return addTimer(interval, callback, false);
  }

  TimerId runAfter(TimeDuration interval, Callback callback) {
    return addTimer(interval, callback, true);
  }

  bool cancel(TimerId id) { return removeTimer(id); }

  bool addIoObject(std::shared_ptr<IoObject> p_object) {
    if (epoll_.DoOperate(Epoller::Operator::kAdd, p_object) == 0) {

      /* if p_object listenning event changed,event loop will know */
      p_object->sig_listenning_event_changed.connect(
          [this, p_weak_ptr = std::weak_ptr(p_object)]() {
            auto p = p_weak_ptr.lock();
            if (p) {
              modifyIoObject(p);
            }
          });

      return true;
    }
    return false;
  }

  bool deleteIoObject(std::shared_ptr<IoObject> p_object) {
    if (epoll_.DoOperate(Epoller::Operator::kDelete, p_object) == 0) {
      return true;
    }
    return false;
  }

private:
  bool modifyIoObject(std::shared_ptr<IoObject> p_object) {
    if (!p_object) {
      return false;
    }
    if (epoll_.DoOperate(Epoller::Operator::kModify, p_object) == 0) {
      return true;
    }
    return false;
  }
  TimerId addTimer(TimeDuration interval, Callback cb, bool one_shot);
  bool removeTimer(TimerId timer);

  std::mutex mutex_;
  /*TODO 将其改为一个析构时关闭资源的结构体 */
  std::unordered_map<int, std::shared_ptr<IoObject>>
      timer_fd_to_info_map_; // guarded by mutex_

  std::atomic_bool quit_{false};
  Epoller epoll_;
};