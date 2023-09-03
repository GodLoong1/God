#include "god/net/InetAddress.h"

#include <arpa/inet.h>

#include <cstring>

#include "god/utils/Logger.h"

namespace god
{

InetAddress::InetAddress() noexcept
{
    ::memset(&addr6_, 0, sizeof(addr6_));
}

InetAddress::InetAddress(uint16_t port,
                         bool loopback,
                         bool ipv6) noexcept
: InetAddress()
{
    if (!ipv6)
    {
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = ::htonl(loopback ?
                                        INADDR_LOOPBACK : INADDR_ANY);
        addr_.sin_port = ::htons(port);
    }
    else
    {
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_addr = loopback ? in6addr_loopback : in6addr_any;
        addr6_.sin6_port = ::htons(port);
    }
}

InetAddress::InetAddress(const std::string_view& ip,
                         uint16_t port,
                         bool ipv6) noexcept
: InetAddress()
{
    if (!ipv6)
    {
        addr_.sin_family = AF_INET;
        addr_.sin_port = ::htons(port);
        if (::inet_pton(AF_INET, ip.data(), &addr_.sin_addr) <= 0)
        {
            LOG_FATAL << strerr();
        }
    }
    else
    {
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = ::htons(port);
        if (::inet_pton(AF_INET6, ip.data(), &addr6_.sin6_addr) <= 0)
        {
            LOG_FATAL << strerr();
        }
    }
}

std::string InetAddress::toIp() const noexcept
{
    char ip[64];
    if (family() == AF_INET)
    {
        if (::inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip)) == nullptr)
        {
            LOG_FATAL << strerr();
        }
    }
    else
    {
        if (::inet_ntop(AF_INET6, &addr6_.sin6_addr, ip, sizeof(ip)) == nullptr)
        {
            LOG_FATAL << strerr();
        }
    }
    return ip;
}

uint16_t InetAddress::toPort() const noexcept
{
    return ::ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const noexcept
{
    return toIp() + ':' + std::to_string(toPort());
}

bool operator==(const InetAddress& lhs, const InetAddress& rhs) noexcept
{
    if (lhs.family() == AF_INET && rhs.family() == AF_INET)
    {
        return lhs.addr_.sin_port == rhs.addr_.sin_port &&
               lhs.addr_.sin_addr.s_addr == rhs.addr_.sin_addr.s_addr;
    }
    else if (lhs.family() == AF_INET6 && rhs.family() == AF_INET6)
    {
        return lhs.addr6_.sin6_port == rhs.addr6_.sin6_port &&
               memcmp(&lhs.addr6_.sin6_addr, &rhs.addr6_.sin6_addr,
                      sizeof(rhs.addr6_.sin6_addr)) == 0;
    }
    return false;
}

} // namespace god