#pragma once
#include <cassert>
#include <errno.h>
#include <memory>
#include <type_traits>
#include <unistd.h>

#include "io_object.h"

class Epoller {
public:
  Epoller() {
    fd_ = epoll_create1(0);
    if (fd_ < 0) {
      throw errno;
    }
  }
  ~Epoller() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }
  enum Operator {
    kAdd = EPOLL_CTL_ADD,
    kModify = EPOLL_CTL_MOD,
    kDelete = EPOLL_CTL_DEL,
  };
  int DoOperate(Operator operation, std::weak_ptr<IoObject> p_weak_object) {
    auto p_object = p_weak_object.lock();
    assert(p_object);

    struct epoll_event evt;
    evt.events = p_object->listenning_events();
    evt.data.fd = p_object->fd();

    auto result = ::epoll_ctl(fd_, operation, p_object->fd(), &evt);
    if (result == 0) {
      if (operation == Operator::kAdd) {
        std::lock_guard<std::mutex> guard(mutex_);
        fd_to_object_map_.emplace(p_object->fd(), std::weak_ptr(p_object));
      } else if (operation == Operator::kDelete) {
        std::lock_guard<std::mutex> guard(mutex_);
        fd_to_object_map_.erase(p_object->fd());
      }
    }
    return result;
  }

  std::vector<std::pair<int,std::shared_ptr<IoObject>>>
  wait(int timeout_ms = -1) { // -1 means wait forever
    static constexpr size_t kMaxObjectSize = 128UL;
    std::vector<std::pair<int,std::shared_ptr<IoObject>>> result;
    std::vector<epoll_event> events_vec;
    events_vec.resize(kMaxObjectSize);
    int ready_n =
        epoll_wait(fd_, events_vec.data(), events_vec.size(), timeout_ms);
    if (ready_n >= 0) {
      events_vec.resize(ready_n);

      for (const auto &ele : events_vec) {
        auto fd = ele.data.fd;
        std::shared_ptr<IoObject> p_object_ptr;
        {
          std::lock_guard<std::mutex> guard(mutex_);
          assert(fd_to_object_map_.find(fd) != fd_to_object_map_.end());
          p_object_ptr = fd_to_object_map_.at(fd).lock();

          if( !p_object_ptr ) {
            fd_to_object_map_.erase(fd);
            continue;
          }
        }
        
        p_object_ptr->setActiveEvent(ele.events);
        result.emplace_back(fd,p_object_ptr);
      }
    }
    return result;
  }

  void DeleteAllInvalidIoObject() {
    std::mutex mutex_;
    auto iter = fd_to_object_map_.begin();
    while (iter != fd_to_object_map_.end()) {
      auto [fd, weak_ptr] = *iter;
      auto p_shared_ptr = weak_ptr.lock();
      if ( !p_shared_ptr )
      {
        iter = fd_to_object_map_.erase(iter);
        assert(0 == ::epoll_ctl(fd, Operator::kDelete, fd, nullptr));
      }
      else {
        iter++;
      }
    }
  }

private:
  std::unordered_map<int, std::weak_ptr<IoObject>>
      fd_to_object_map_; // guarded by mutex_
  std::mutex mutex_;

  int fd_ = -1;
};