#pragma once
#include <atomic>
#include <cassert>
#include <functional>
#include <memory>

#include "event_loop.h"
#include "socket.h"
#include "work_thread.h"

class UdpServer {
public:
  UdpServer(InetAddress local_address,EventLoop * event_loop)
      : event_loop_(event_loop),local_socket_(Socket::CreateUdpSocket()),
        local_address_(std::move(local_address)),
        p_io_object_(std::make_shared<IoObject>(local_socket_.getFd()))
{
    local_socket_.bind(local_address);

    p_io_object_->EnableEvent(IoEventType::kReadEvent);
    p_io_object_->sig_new_read_event.connect([this]() { OnReadableEvent(); });
    p_io_object_->sig_new_write_event.connect([this]() { OnWritableEvent(); });
    assert(event_loop_->addIoObject(p_io_object_));
  }
  ~UdpServer() = default;

  const InetAddress &getLocalAddresss() const { return local_address_; }

  using OnMessageCallback = std::function<void(InetAddress, Buffer &)>;

  void SetOnMessageCallback(OnMessageCallback callback) {
    on_message_callback_ = std::move(callback);
  }

  void SendTo(const InetAddress &address, std::shared_ptr<std::string> data) {
    event_loop_->runAfter(TimeDuration(0), [=]() {
      if (send_buffer_vector_.empty()) {
        p_io_object_->EnableEvent(IoEventType::kWriteEvent);
      }

      send_buffer_vector_.push_back(std::make_pair(address, data));
    });
  }

  void SendTo(const InetAddress &address, Buffer & data) {
    auto p_data = std::make_shared<std::string>(data.toStringView());
    SendTo(address,p_data);
  }
  void RecvFrom() = delete;

  void EnableWriting() {
    p_io_object_->EnableEvent(IoEventType::kWriteEvent);
  }

  void DisableWriting() {
    p_io_object_->DisableEvent(IoEventType::kWriteEvent);
  }

private:
  void OnReadableEvent() {
    event_loop_->runAfter(TimeDuration(0), [this]() {
      if (!on_message_callback_) {
        return;
      }

      InetAddress peer_address;
      int error_code = 0;
      do {
        Buffer buffer;
        std::tie(error_code, peer_address) = local_socket_.recv(buffer);
        if (error_code == 0) {
          on_message_callback_(std::move(peer_address), buffer);
        } else {
          if (error_code != EAGAIN || error_code != EWOULDBLOCK) {
            printf("error_code %d\n", error_code);
          }
        }

      } while (error_code == 0);

    });
  }
  void OnWritableEvent() {
    event_loop_->runAfter(TimeDuration(0), [this]() {

      auto iter = send_buffer_vector_.begin();
      while (iter != send_buffer_vector_.end()) {
        auto &addr = iter->first;
        auto &buffer = iter->second;

        assert(buffer->size() <= 1500); //发送udp数据,每次发送的大小有限制

        auto ret =
            ::sendto(local_socket_.getFd(), buffer->c_str(), buffer->size(), 0,
                     addr.getSockAddr(), addr.GetSockStrLen());
        if (ret > 0) {
          assert(buffer->size() == ret); // 希望全部发完

          iter = send_buffer_vector_.erase(iter);
        } else {
          if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
          } else {
            // message
            printf("sendto error in udp server,ret %ld, errno %d", ret, errno);

            assert(false); // 有错误原因的话，直接崩溃
          }
        }
      }

      /* if there is no other data to write , stop listenning writable event */
      if (send_buffer_vector_.empty()) {
        p_io_object_->DisableEvent(IoEventType::kWriteEvent);
      }
    });
  }

  EventLoop *event_loop_;
  OnMessageCallback on_message_callback_;

  Socket local_socket_;
  std::shared_ptr<IoObject> p_io_object_; // for udp socket

  InetAddress local_address_;
  std::vector<std::pair<InetAddress, std::shared_ptr<std::string>>>
      send_buffer_vector_;
};