#pragma once

#include "pch.hpp"

#include "ServerInfo.hpp"

constexpr auto DEFAULT_MASTERLIST = "https://masterlist.lc-mp.org";

class MyFrame;

enum class ListViewTab : uint8_t;

enum class MasterListRequestType : uint8_t
{
    ALL_SERVERS,
    OFFICIAL_SERVERS,
};

class BrowserSettings {
public:
    BrowserSettings()
        : masterlist(DEFAULT_MASTERLIST)
        , windowed(false)
        , showConsole(false)
    {}

    String nickname;
    Path   gamePath;
    String proxy;
    String masterlist;
    bool   windowed;
    bool   showConsole;
};

struct BrowserRequestResult
{
    CURLcode code;
    int      httpCode;
    String   errorMessage;
};

class Browser {
public:
    MyFrame*        m_frame;
    BrowserSettings m_settings;

    std::unordered_map<ListViewTab, std::unordered_map<String, std::shared_ptr<ServerInfo>>> m_serversList;

    Browser(MyFrame*);
    ~Browser();

    BrowserRequestResult MakeHttpRequest(const String& url, String& data) const;

    void RequestMasterList(MasterListRequestType type);

    void QueryServer(std::shared_ptr<ServerInfo> serverInfo);
    void LaunchGame(const String& host, uint16_t port);
    void SaveSettings();
    void LoadSettings();
    void AddToFavorites(const ServerHost& host);
    void RemoveFromFavorites(const ServerHost& host);
};

extern Unique<Browser> gBrowser;
