#pragma once
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <string_view>
#include <vector>

class Buffer {
public:
  static constexpr std::size_t kDefaultCapability = 1024UL;
  Buffer() { data_.reserve(kDefaultCapability); }

  Buffer(const Buffer &rhs) = default;
  Buffer &operator=(const Buffer &rhs) = default;

  Buffer(Buffer &&rhs) = default;
  Buffer &operator=(Buffer &&rhs) = default;

  ~Buffer() = default;

  Buffer &operator<<(char ch) {
    write(reinterpret_cast<const uint8_t *>(&ch), sizeof(ch));
    return (*this);
  }

  Buffer &operator<<(uint8_t n) {
    write(reinterpret_cast<const uint8_t *>(&n), sizeof(n));
    return (*this);
  }
  Buffer &operator<<(uint16_t n) {
    write(reinterpret_cast<const uint8_t *>(&n), sizeof(n));
    return (*this);
  }

  Buffer &operator<<(uint32_t n) {
    write(reinterpret_cast<const uint8_t *>(&n), sizeof(n));
    return (*this);
  }
  Buffer &operator<<(uint64_t n) {
    write(reinterpret_cast<const uint8_t *>(&n), sizeof(n));
    return (*this);
  }

  Buffer &operator<<(std::string_view str) {
    write(reinterpret_cast<const uint8_t *>(str.data()), str.size());
    return (*this);
  }

  void write(const uint8_t *p_data_start, size_t n_bytes) {
    assert(p_data_start);

    if (writableSize() < n_bytes) { // allocate space at first
      data_.resize(data_.size() + n_bytes - writableSize());
    }

    auto dst_start_iter = data_.begin() + index_for_writing_;

    std::copy_n(p_data_start, n_bytes, dst_start_iter);

    index_for_writing_ += n_bytes;
    return;
  }

  void write(uint8_t c, size_t n_bytes) {
    auto dst_start_iter = data_.begin() + index_for_writing_;
    auto writable_size = writableSize();

    auto remaining_bytes = n_bytes;

    if (remaining_bytes > writable_size) {
      data_.insert(data_.end(), remaining_bytes - writable_size, c);
      remaining_bytes = writable_size;
    }

    std::fill(dst_start_iter, dst_start_iter + remaining_bytes, c);
    // remaining_bytes = 0;

    index_for_writing_ += n_bytes;
    return;
  }

  Buffer &operator>>(uint8_t &n) {
    read(reinterpret_cast<uint8_t *>(&n), sizeof(n));
    return (*this);
  }

  Buffer &operator>>(uint16_t &n) {
    read(reinterpret_cast<uint8_t *>(&n), sizeof(n));
    return (*this);
  }

  Buffer &operator>>(uint32_t &n) {
    read(reinterpret_cast<uint8_t *>(&n), sizeof(n));
    return (*this);
  }

  std::string_view toStringView() const {
    return {reinterpret_cast<const char *>(&data_[index_for_reading_]),
            readableSize()};
  }

  std::pair<const uint8_t *, size_t> getReadIndexAndSize() const {
    return {&data_[index_for_reading_], readableSize()};
  }

  size_t read(uint8_t *p_data_out, size_t n_bytes) {
    assert(p_data_out);

    auto read_bytes = peek(p_data_out, n_bytes);
    index_for_reading_ += read_bytes;
    return read_bytes;
  }

  uint8_t peekUint8() const {
    uint8_t result;
    peek(reinterpret_cast<uint8_t *>(&result), sizeof(result));
    return result;
  }

  uint16_t peekUint16() const {
    uint16_t result;
    peek(reinterpret_cast<uint8_t *>(&result), sizeof(result));
    return result;
  }

  uint32_t peekUin32() const {
    uint32_t result;
    peek(reinterpret_cast<uint8_t *>(&result), sizeof(result));
    return result;
  }

  size_t peek(uint8_t *p_data_out, size_t n_bytes) const {
    assert(p_data_out);
    auto result = std::min(readableSize(), n_bytes);
    auto peek_start_iter = data_.begin() + index_for_reading_;
    std::copy(peek_start_iter, peek_start_iter + result, p_data_out);
    return result;
  }

  size_t readableSize() const { return data_.size() - index_for_reading_; }
  size_t writableSize() const { return data_.size() - index_for_writing_; }

  void clear() {
    data_.clear();
    index_for_writing_ = 0;
    index_for_reading_ = 0;
  }

  std::size_t getCurrentWriteIndex() const { return index_for_writing_; }
  std::size_t getCurrentReadIndex() const { return index_for_reading_; }

  void seekWriteIndex(size_t new_position) {
    assert(new_position <= data_.size());
    index_for_writing_ = new_position;
  }

  void seekReadIndex(size_t new_position) {
    assert(new_position <= data_.size());
    index_for_reading_ = new_position;
  }

private:
  std::vector<uint8_t> data_;
  std::size_t index_for_writing_ = 0;
  std::size_t index_for_reading_ = 0;
};