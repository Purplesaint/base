#pragma once
#include "io_base.h"
#include "base/sig_slot.h"
#include <functional>

class IoObject {
public:
  using EventCallback = std::function<void()>;
  IoObject(int fd) : fd_(fd) {}
  ~IoObject() = default;

  nod::unsafe_signal<void()> sig_listenning_event_changed;

  nod::unsafe_signal<void()> sig_new_read_event;
  nod::unsafe_signal<void()> sig_new_write_event;
  nod::unsafe_signal<void()> sig_new_close_event;
  nod::unsafe_signal<void()> sig_new_error_event;

  int fd() const { return fd_; }
  int listenning_events() const { return listenning_events_; }
  int active_events() const { return active_events_; }
  void setActiveEvent(unsigned int event) { active_events_ = event; }
  void EnableEvent(IoEventType type)
  {
    listenning_events_ |= type;
    sig_listenning_event_changed();
  }
  void DisableEvent(IoEventType type) {
    listenning_events_ &= (~type);
    sig_listenning_event_changed();
  }
  bool IsEventEnabled(IoEventType type) const {
    return (listenning_events_ & type) == type;
  }
  bool IsEventDisabled(IoEventType type) const {
    return !IsEventDisabled(type);
  }

  void TriggerEvent() {
    auto events = active_events_;
    active_events_ = IoEventType::kNone;
    if ((events & EPOLL_EVENTS::EPOLLHUP) &&
        !(events & EPOLL_EVENTS::EPOLLIN))
    { // 防止数据没读完
      sig_new_close_event();
    }

    if (events & (EPOLL_EVENTS::EPOLLERR)) {
      sig_new_error_event();
    }

    if (events & (EPOLL_EVENTS::EPOLLIN | EPOLL_EVENTS::EPOLLPRI |
                  EPOLL_EVENTS::EPOLLRDHUP)) {
      sig_new_read_event();
    }

    if (events & (EPOLL_EVENTS::EPOLLOUT)) {
      sig_new_write_event();
    }
  }

private:
  int fd_ = -1; /* this object do not own this fd */
  unsigned int listenning_events_ = IoEventType::kNone;
  unsigned int active_events_ = IoEventType::kNone;
};