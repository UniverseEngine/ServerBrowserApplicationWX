#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

class ServerInfo;
class ServerHost;

enum class ServerListType : uint8_t
{
    FAVORITES = 0,
    INTERNET,
    OFFICIAL,
    LAN
};

enum class ListColumnID : int
{
    KEY = 0,
    VALUE,

    ICON = 0,
    NAME,
    PING,
    PLAYERS,
    VERSION,
    GAMEMODE
};

class MyFrame : public wxFrame {
public:
    MyFrame();

    void AppendServer(ServerListType tab, ServerInfo* serverInfo);
    void UpdateServerColumn(ServerListType tab, const ServerHost& host, ListColumnID id, const std::string& data);
    void RemoveServer(ServerListType tab, const ServerHost& host);
    void RemoveAllServers(ServerListType tab);

    ServerListType GetCurrentTab();
    void        SetCurrentTab(ServerListType tab);

private:
    wxNotebook*                                  m_notebook;
    std::unordered_map<ServerListType, wxListView*> m_listViews;
    wxListView*                                  m_serverRulesListView;
    wxListBox*                                   m_playerListbox;
    wxStaticText*                                m_serverInfoName;
    wxStaticText*                                m_serverInfoHost;
    wxStaticText*                                m_serverInfoPlayers;
    wxStaticText*                                m_serverInfoPing;
    wxListItem                                   m_selectedServerItem;

    long FindItemByData(ServerListType tab, const ServerHost& data);

    void AddTab(ServerListType tab, const std::string& name);

    void OnPageChange(wxBookCtrlEvent& event);
    void OnItemSelected(wxListEvent& event);
    void OnItemActivated(wxListEvent& event);
    void OnDeleteItem(wxListEvent& event);
    void OnRightClickItem(wxListEvent& event);

    void OnOpenCrashFolder(wxCommandEvent& event);
    void OnOpenLogFolder(wxCommandEvent& event);
    void OnAddServer(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
};
