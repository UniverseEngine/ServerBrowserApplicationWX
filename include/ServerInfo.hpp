#include "pch.hpp"

#include "ServerHost.hpp"

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

    ServerInfo(String ip, uint16_t port)
        : m_host(ServerHost(ip, port))
        , m_name("(Waiting server response...)")
        , m_maxPlayers(0)
        , m_passworded(false)
        , m_lastPlayed(0)
        , m_ping(9999)
        , m_online(false)
    {}

    struct PlayerInfo
    {
        std::string playerName;
    };

    ServerHost         m_host;
    String             m_name;
    uint32_t           m_maxPlayers;
    bool               m_passworded;
    String             m_gamemode;
    String             m_version;
    Vector<PlayerInfo> m_players;

    time_t    m_lastPlayed;
    long long m_ping;
    bool      m_online;
};
