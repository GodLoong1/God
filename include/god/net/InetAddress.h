#ifndef GOD_NET_INETADDRESS_H
#define GOD_NET_INETADDRESS_H

#include <netinet/in.h>

#include <string>
#include <string_view>

namespace god
{

/// Tcp地址
class InetAddress
{
public:
    InetAddress() noexcept;

    explicit InetAddress(uint16_t port,
                         bool loopback = false,
                         bool ipv6 = false) noexcept;

    InetAddress(const std::string_view& ip,
                uint16_t port,
                bool ipv6 = false) noexcept;

    int family() const noexcept
    {
        return addr_.sin_family;
    }

    std::string toIp() const noexcept;
    uint16_t toPort() const noexcept;
    std::string toIpPort() const noexcept;

    const struct sockaddr* getSockAddr() const noexcept
    {
        return reinterpret_cast<const struct sockaddr*>(&addr6_);
    }

    struct sockaddr* getSockAddr() noexcept
    {
        return reinterpret_cast<struct sockaddr*>(&addr6_);
    }

    constexpr socklen_t getSockLen() const noexcept
    {
        return sizeof(addr6_);
    }

    friend bool operator==(const InetAddress& lhs,
                           const InetAddress& rhs) noexcept;

private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};

} // namespace god

#endif