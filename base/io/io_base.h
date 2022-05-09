#pragma once
#include <sys/epoll.h>
#include <type_traits>
enum IoEventType : std::underlying_type_t<EPOLL_EVENTS>
{
    kNone = 0,
    kReadEvent = EPOLL_EVENTS::EPOLLIN | EPOLL_EVENTS::EPOLLPRI,
    kWriteEvent = EPOLL_EVENTS::EPOLLOUT,
};