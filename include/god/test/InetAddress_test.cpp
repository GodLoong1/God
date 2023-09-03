#include "god/net/InetAddress.h"
#include <iostream>

using namespace god;

int main()
{
    InetAddress addr1(1314, false, false);
    InetAddress addr2(1314, true, false);
    InetAddress addr3(1314, true, true);
    InetAddress addr4(1314, false, true);

    InetAddress addr5("10.0.4.16", 9898, false);
    InetAddress addr6("fe80::5054:ff:fe8d:a4e", 9898, true);

    std::cout << addr1.toIpPort() << std::endl;
    std::cout << addr2.toIpPort() << std::endl;
    std::cout << addr3.toIpPort() << std::endl;
    std::cout << addr4.toIpPort() << std::endl;
    std::cout << addr5.toIpPort() << std::endl;
    std::cout << addr6.toIpPort() << std::endl;
}