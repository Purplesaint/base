#pragma once

#include "event_loop.h"
#include "sig_slot.h"
#include <memory>
#include <string>
#include <string_view>
#include <thread>

class WorkThread {
public:
  using Callback = std::function<void()>;
  WorkThread(std::string name) : name_(std::move(name)) {
    p_event_loop_ = std::make_unique<EventLoop>();
  }
  ~WorkThread() { stop();
    join();
  }

  nod::unsafe_signal<void()> sig_started;
  nod::unsafe_signal<void()> sig_stoped;
  void start() {
    if (!is_running_) {
      thread_ = std::thread([this]() { routine(); });

      while ( !is_running_ ) {
        continue;
      }
    }
  }

  void stop() {
    if (is_running_) {
      p_event_loop_->quit();
    }
  }
  void join() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }
  std::thread::id getId() const { return thread_.get_id(); }
  EventLoop *getLoop() const { return p_event_loop_.get(); }

private:
  void routine() {
    is_running_ = true;
    pthread_setname_np(thread_.native_handle(), name_.c_str());
    sig_started();

    // do routine
    p_event_loop_->loop();
    sig_stoped();

    is_running_ = false;
  }

  std::thread thread_;
  std::string name_;

  std::unique_ptr<EventLoop> p_event_loop_;
  std::atomic_bool is_running_ {false};
};