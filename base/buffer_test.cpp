#include "gtest/gtest.h"
#include "buffer.h"

TEST(TestWriteAndRead,AllCases) {
 {
    Buffer buffer;
    uint8_t n = 2,tmp;
    buffer << n;
    EXPECT_EQ(sizeof(n), buffer.readableSize());

    buffer >> tmp;

    EXPECT_EQ(n, tmp);
 }

 {
    Buffer buffer;
    uint16_t n = 2,tmp;
    buffer << n;
    EXPECT_EQ(sizeof(n), buffer.readableSize());

    buffer >> tmp;

    EXPECT_EQ(n, tmp);
 }

 {
    Buffer buffer;
    uint32_t n = 2,tmp;
    buffer << n;
    EXPECT_EQ(sizeof(n), buffer.readableSize());

    buffer >> tmp;

    EXPECT_EQ(n, tmp);
 }
 
 {
    Buffer buffer;

    uint32_t a = 1, b = 2, c = 3;
    buffer << a << b << c;

    uint32_t tmp;
    buffer >> tmp;
    EXPECT_EQ(tmp, a);

    buffer >> tmp;
    EXPECT_EQ(tmp, b);

    buffer >> tmp;
    EXPECT_EQ(tmp, c);
 }
 
}