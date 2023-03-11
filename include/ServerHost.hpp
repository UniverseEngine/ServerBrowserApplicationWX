#include "pch.hpp"

class ServerHost {
public:
    String   m_ip;
    uint16_t m_port;

    ServerHost()
        : m_port(0)
    {}

    ServerHost(String ip, uint16_t port)
        : m_ip(ip)
        , m_port(port)
    {}

    ServerHost(String host)
    {
        auto split = host.find_first_of(":");
        if (split != std::string::npos)
        {
            m_ip   = host.substr(0, host.find_first_of(":"));
            m_port = std::atoi(host.substr(split + 1).c_str());
        }
        else
        {
            m_ip  = host;
            m_port = 7777;
        }
    }

    String ToString() const
    {
        return std::format("{}:{}", m_ip, m_port);
    }

    static bool IsValid(const String& host)
    {
        if (host.find(":") != host.find_last_of(":"))
            return false;

        String ip = host.substr(0, host.find_first_of(":"));

        struct sockaddr_in sa;
        int                result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
        return result != 0;
    }

    bool operator==(const ServerHost& h)
    {
        return h.m_ip == m_ip && h.m_port == m_port;
    }
};