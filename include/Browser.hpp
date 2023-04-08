#pragma once

#include "pch.hpp"

#include "ServerInfo.hpp"

class MyFrame;

enum class ListViewTab : uint8_t;

struct _settings
{
    String nickname;
    Path   gamePath;
    String proxy;
    bool   windowed;
    bool   freeCam;
    bool   showConsole;
};

class Browser {
public:
    HWND      m_hwnd;
    MyFrame*  m_frame;
    CURL*     m_curl;
    _settings m_settings;
    String    m_masterlistURL;

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