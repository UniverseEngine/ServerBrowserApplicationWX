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

    using ServerMap = UnorderedMap<String, ServerInfo>;
    ServerMap m_serversList;
    ServerMap m_favoriteList;

    Browser(MyFrame*);
    ~Browser();

    ServerMap& GetServerListFromTab(ListViewTab tab);

    BrowserRequestResult MakeHttpRequest(const String& url, String& data) const;

    void RequestMasterList(MasterListRequestType type);

    void QueryServer(ServerInfo& serverInfo, bool updatePlayerList = false);
    bool ParseMasterListResponse(String jsonStr);
    bool LaunchGame(const String& host, uint16_t port);
    void SaveSettings();
    void LoadSettings();
    void AddToFavorites(const ServerHost& host);
    void RemoveFromFavorites(const ServerHost& host);
};

extern Unique<Browser> gBrowser;
