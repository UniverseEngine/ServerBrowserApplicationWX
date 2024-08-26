#include "pch.hpp"

#include <wx/listctrl.h>
#include <wx/notebook.h>

class ServerInfo;
class ServerHost;

enum class ListViewTab : uint8_t
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

    void AppendServer(ListViewTab tab, ServerInfo* serverInfo);
    void UpdateServerColumn(ListViewTab tab, const ServerHost& host, ListColumnID id, const String& data);
    void RemoveServer(ListViewTab tab, const ServerHost& host);
    void RemoveAllServers(ListViewTab tab);

    ListViewTab GetCurrentTab();
    void        SetCurrentTab(ListViewTab tab);

private:
    wxNotebook*                            m_notebook;
    UnorderedMap<ListViewTab, wxListView*> m_listViews;
    wxListView*                            m_serverRulesListView;
    wxListBox*                             m_playerListbox;
    wxStaticText*                          m_serverInfoName;
    wxStaticText*                          m_serverInfoHost;
    wxStaticText*                          m_serverInfoPlayers;
    wxStaticText*                          m_serverInfoPing;
    wxListItem                             m_selectedServerItem;

    long FindItemByData(ListViewTab tab, const ServerHost& data);

    void AddTab(ListViewTab tab, const String& name);

    void OnPageChange(wxBookCtrlEvent& event);
    void OnItemSelected(wxListEvent& event);
    void OnItemActivated(wxListEvent& event);
    void OnDeleteItem(wxListEvent& event);
    void OnRightClickItem(wxListEvent& event);

    void OnAddServer(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
};
