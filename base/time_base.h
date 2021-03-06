#pragma once
#include <chrono>
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
using TimeDuration = std::chrono::milliseconds;

static inline Timestamp now() { return std::chrono::system_clock::now(); }