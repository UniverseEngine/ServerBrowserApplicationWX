#pragma once

#include "pch.hpp"

#include "ServerInfo.hpp"

#define DEFAULT_MASTERLIST "https://masterlist.lc-mp.org"

class MyFrame;

enum class ListViewTab : uint8_t;

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

    void     ReadFromSocket();
    CURLcode Request(String url, String& data);
    bool     ParseMasterListResponse(String jsonStr);
    bool     LaunchGame(String host, uint16_t port);
    void     SaveSettings();
    void     LoadSettings();
    void     AddToFavorites(const ServerHost& host);
    void     RemoveFromFavorites(const ServerHost& host);
};

extern Unique<Browser> gBrowser;