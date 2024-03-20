#pragma once

#include <Universe.hpp>

#include "ServerInfo.hpp"

#include <curl/curl.h>

#define DEFAULT_MASTERLIST "https://masterlist.lc-mp.org"

using namespace Universe;

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
    HWND            m_hwnd;
    MyFrame*        m_frame;
    CURL*           m_curl;
    BrowserSettings m_settings;

    using ServerMap = UnorderedMap<String, ServerInfo>;
    ServerMap m_serversList;
    ServerMap m_favoriteList;

    SOCKET m_socket;

    Browser(MyFrame*);
    ~Browser();

    ServerMap& GetServerListFromTab(ListViewTab tab);

    BrowserRequestResult Request(String url, String& data);

    void RequestMasterList(MasterListRequestType type);

    void QueryServer(ServerInfo& serverInfo);
    void ReadFromSocket();
    bool ParseMasterListResponse(String jsonStr);
    bool LaunchGame(const String& host, uint16_t port);
    void SaveSettings();
    void LoadSettings();
    void AddToFavorites(const ServerHost& host);
    void RemoveFromFavorites(const ServerHost& host);
};

extern Unique<Browser> gBrowser;
