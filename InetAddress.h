#pragma once

#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>

namespace mymuduo
{
//封装socket地址类型
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in &addr);

    std::string toIP() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in *getSockAddr() const;
    void setSockAddr(const sockaddr_in &addr);

private:
    sockaddr_in m_addr;
};
} // namecpace mymuduo