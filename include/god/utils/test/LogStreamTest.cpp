#include "god/utils/LogStream.h"

#include <iostream>
#include <string>
#include <string_view>

int main()
{
    god::LogBuffer<5> buf1;
    buf1.append("1234", 4);

    std::cout << "buf1 data: " << buf1.data() << "\n";
    std::cout << "buf1 size: " << buf1.size() << "\n";
    std::cout << "buf1 capacity: " << buf1.capacity() << "\n";


    god::LogBuffer<5> buf2;
    buf2.append("12345", 5);

    std::cout << "==============================\n";
    std::cout << "buf2 data: " << buf2.data() << "\n";
    std::cout << "buf2 size: " << buf2.size() << "\n";
    std::cout << "buf2 capacity: " << buf2.capacity() << "\n";


    god::LogStream stream;
    stream << 1 << 2.2 << 'c' << "???";

    std::cout << "==============================\n";
    std::cout << "stream data: " << stream.data() << "\n";
    std::cout << "stream size: " << stream.size() << "\n";

    stream << 100 << 2.233 << 'c' << "!!!";

    std::cout << "==============================\n";
    std::cout << "stream data: " << stream.data() << "\n";
    std::cout << "stream size: " << stream.size() << "\n";

    god::LogStream stream2;
    stream2 << "wfafasdfasdfasdfasdf";

    stream << std::string("string") << std::string_view(" string_view")
           << buf1 << " " << buf2 << false << true << &printf
           << stream2;

    std::cout << "==============================\n";
    std::cout << "stream data: " << stream.data() << "\n";
    std::cout << "stream size: " << stream.size() << "\n";

}