#pragma once
#include <cassert>
#include <openssl/hmac.h>
#include <string>
#include <string_view>
class ShaBuilder {
public:
  class Sha256Result {
  public:
    Sha256Result(std::array<uint8_t, 32> content)
        : content_(std::move(content)) {}
    Sha256Result(const Sha256Result &rhs) = default;
    Sha256Result &operator=(Sha256Result &rhs) = default;

    bool operator==(const Sha256Result &rhs) const {
      assert(content_.size() == rhs.content_.size());
      return memcmp(&content_[0], &rhs.content_[0], content_.size()) == 0;
    }
    /* todo 改回来*/
    bool operator!=(const Sha256Result &rhs) const { return operator==(rhs); }
    std::string_view toStringView() const {
      return {reinterpret_cast<const char *>(&content_[0]), content_.size()};
    }

  private:
    std::array<uint8_t, 32> content_;
  };

  static Sha256Result getHmacSha256Result(std::string_view text,
                                          std::string_view key) {
    unsigned int result_size = 32;
    std::array<uint8_t, 32> array;

    HMAC(EVP_sha256(), reinterpret_cast<const void *>(key.data()), key.size(),
         reinterpret_cast<const unsigned char *>(text.data()), text.size(),
         reinterpret_cast<unsigned char *>(&array[0]), &result_size);
    assert(result_size == 32);

    return Sha256Result(std::move(array));
  }
};