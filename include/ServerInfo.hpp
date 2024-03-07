#include "ServerHost.hpp"

class ServerInfo {
public:
    ServerInfo()
        : m_name("(Waiting server response...)")
        , m_players(0)
        , m_maxPlayers(0)
        , m_passworded(false)
        , m_lastPlayed(0)
        , m_lastPingRecv(0)
        , m_ping(0)
        , m_online(false)
    {}

    ServerInfo(String ip, uint16_t port)
        : m_host(ServerHost(ip, port))
        , m_name("(Waiting server response...)")
        , m_players(0)
        , m_maxPlayers(0)
        , m_passworded(false)
        , m_lastPlayed(0)
        , m_lastPingRecv(0)
        , m_ping(0)
        , m_online(false)
    {}

    ServerHost     m_host;
    String         m_name;
    uint32_t       m_players;
    uint32_t       m_maxPlayers;
    bool           m_passworded;
    String         m_gamemode;
    String         m_version;
    Vector<String> m_playerList;

    time_t   m_lastPlayed;
    uint64_t m_lastPingRecv;
    uint32_t m_ping;
    bool     m_online;
};