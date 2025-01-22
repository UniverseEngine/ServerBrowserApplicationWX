#pragma once

#include "ServerHost.hpp"

#include <string>
#include <unordered_map>
#include <vector>

using Rules   = std::unordered_map<std::string, std::string>;
using Players = std::vector<std::string>;

class ServerInfo {
public:
    ServerInfo()
        : m_name("(Waiting server response...)")
        , m_maxPlayers(0)
        , m_passworded(false)
        , m_ping(9999)
        , m_lastPlayed(0)
        , m_online(false)
    {}

    ServerInfo(std::string ip, uint16_t port)
        : m_host(ServerHost(ip, port))
        , m_name("(Waiting server response...)")
        , m_maxPlayers(0)
        , m_passworded(false)
        , m_lastPlayed(0)
        , m_ping(9999)
        , m_online(false)
    {}

    ServerHost  m_host;
    std::string m_name;
    uint32_t    m_maxPlayers;
    bool        m_passworded;
    std::string m_gamemode;
    std::string m_version;
    Players     m_players;
    Rules       m_rules;

    time_t    m_lastPlayed;
    long long m_ping;
    bool      m_online;

    void SetAsOffline()
    {
        m_online = false;
        m_ping   = 9999;
    }
};
