#include "god/utils/LogStream.h"

#include <iostream>
#include <string>

struct Test
{
    const char* data() const { return {}; }
    size_t size() const { return {}; }
};

int main()
{
    god::LogStream<30> buf;

    bool b1 = true;
    bool b2 = false;
    char c = 'c';
    int8_t i8 = -8;
    uint8_t u8 = 8;
    int16_t i16 = -16;
    uint16_t u16 = 16;
    int32_t i32 = -32;
    uint32_t u32 = 32;
    int64_t i64 = -64;
    uint64_t u64 = 64;
    float f32 = -32.32;
    double f64 = -64.64;
    long double f128 = -128.128;
    // char* str1 = "我的char星呢";
    const char* str = "test";
    const void* ptr = &buf;
    Test test;

    buf << b1;
    buf << b2;
    buf << c;
    buf << i8;
    buf << u8;
    buf << i16;
    buf << u16;
    buf << i32;
    buf << u32;
    buf << i64;
    buf << u64;
    buf << f32;
    buf << f64;
    buf << f128;
    // buf << str1;
    buf << str;
    buf << ptr;
    buf << test;

    std::cout << buf.size() << std::endl;
    std::cout << buf.data() << std::endl;
}