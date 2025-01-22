#pragma once

#include <string>
#include <format>

class ServerHost {
public:
    std::string m_ip;
    uint16_t    m_port;

    ServerHost()
        : m_port(0)
    {}

    ServerHost(std::string ip, uint16_t port)
        : m_ip(ip)
        , m_port(port)
    {}

    ServerHost(std::string host)
    {
        auto split = host.find_first_of(":");
        if (split != std::string::npos)
        {
            m_ip   = host.substr(0, host.find_first_of(":"));
            m_port = std::atoi(host.substr(split + 1).c_str());
        }
        else
        {
            m_ip   = host;
            m_port = 7777;
        }
    }

    std::string ToString() const
    {
        return std::format("{}:{}", m_ip, m_port);
    }

    bool operator==(const ServerHost& h)
    {
        return h.m_ip == m_ip && h.m_port == m_port;
    }
};
