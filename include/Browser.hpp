#pragma once

#include "ServerInfo.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <curl/curl.h>

static std::string gDefaultMasterlistUrl = "https://masterlist.lc-mp.org";

class MyFrame;

enum class ServerListType : uint8_t;

enum class MasterListType : uint8_t
{
    SERVERS,
    OFFICIAL,
};

class BrowserSettings {
public:
    BrowserSettings()
        : masterlist(gDefaultMasterlistUrl)
        , windowed(false)
        , showConsole(false)
    {}

    std::string           nickname;
    std::filesystem::path gamePath;
    std::string           proxy;
    std::string           masterlist;
    bool                  windowed;
    bool                  showConsole;
};

class Browser {
public:
    MyFrame*        m_frame;
    BrowserSettings m_settings;

    std::unordered_map<ServerListType, std::unordered_map<std::string, std::shared_ptr<ServerInfo>>> m_serversList;

    Browser(MyFrame* frame)
        : m_frame(frame) {}

    void GetServersFromMasterlist(ServerListType type);

    void QueryServer(std::shared_ptr<ServerInfo> serverInfo);
    void LaunchGame(const std::string& host, uint16_t port);
    void SaveSettings();
    void LoadSettings();
    void AddToFavorites(const ServerHost& host);
    void RemoveFromFavorites(const ServerHost& host);
};

extern std::unique_ptr<Browser> gBrowser;
