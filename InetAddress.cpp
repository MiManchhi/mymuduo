#include <strings.h>
#include <string.h>
#include <stdexcept>

#include "InetAddress.h"

namespace mymuduo
{
InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    
    // 处理空IP为INADDR_ANY
    if (ip.empty()) {
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        // 检查inet_pton返回值
        if (inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) <= 0) {
            throw std::invalid_argument("Invalid IPv4 address: " + ip);
        }
    }
}
InetAddress::InetAddress(const sockaddr_in &addr) : m_addr(addr)
{

}

std::string InetAddress::toIP() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    return buf;
}
std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ntohs(m_addr.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}
uint16_t InetAddress::toPort() const
{
    return ntohs(m_addr.sin_port);
}

const sockaddr_in *InetAddress::getSockAddr() const
{
    return &m_addr;
}
void InetAddress::setSockAddr(const sockaddr_in &addr)
{
    m_addr = addr;
}
} //namespace mymuduo

// #include <iostream>
// int main()
// {
//     mymuduo::InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;

//     return 0;
// }