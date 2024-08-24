
#include "pch.hpp"

#include <wx/clipbrd.h>

#include "dialogs/AboutDialog.hpp"
#include "dialogs/AddServerDialog.hpp"
#include "dialogs/SettingsDialog.hpp"

#include "MyFrame.hpp"

#include "Browser.hpp"
#include <thread>

enum
{
    ID_ADDSERVER = 1,
    ID_SETTINGS
};

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Server Browser", wxDefaultPosition, wxSize(1000, 650),
          wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
    SetIcon(wxICON(IDI_APPICON));

    auto menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    auto menuServers = new wxMenu;
    menuServers->Append(ID_ADDSERVER, "Add Server");

    auto menuTools = new wxMenu;
    menuTools->Append(ID_SETTINGS, "Settings");

    auto menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    auto menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuServers, "&Servers");
    menuBar->Append(menuTools, "&Tools");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();

    Bind(wxEVT_MENU, &MyFrame::OnAddServer, this, ID_ADDSERVER);
    Bind(wxEVT_MENU, &MyFrame::OnSettings, this, ID_SETTINGS);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto mainPanel = new wxPanel(this, wxID_ANY);
    mainPanel->SetSizer(sizer);

    {
        auto hbox = new wxBoxSizer(wxHORIZONTAL);
        {
            m_notebook = new wxNotebook(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(800, 400));

            AddTab(ListViewTab::FAVORITES, "Favorites");
            AddTab(ListViewTab::INTERNET, "Internet");
            AddTab(ListViewTab::OFFICIAL, "Official");
            // AddTab(ListViewTab::LAN, "LAN");

            m_notebook->SetSelection(0);
            m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MyFrame::OnPageChange, this);

            hbox->Add(m_notebook);
        }
        {
            auto playerBoxSizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel, "Players");

            m_playerListbox = new wxListBox(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(160, 368));

            playerBoxSizer->Add(m_playerListbox, 1, wxALL | wxEXPAND, 5);

            hbox->Add(playerBoxSizer);
        }

        sizer->Add(hbox, 1);
    }

    {
        auto hbox = new wxBoxSizer(wxHORIZONTAL);
        {
            auto serverRulesBoxSizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel, "Server Rules");

            m_serverRulesListView = new wxListView(mainPanel);
            m_serverRulesListView->InsertColumn((int)ListColumnID::KEY, "Key", 0, 160);
            m_serverRulesListView->InsertColumn((int)ListColumnID::VALUE, "Value", 0, 740);

            serverRulesBoxSizer->Add(m_serverRulesListView, 1, wxALL | wxEXPAND, 5);

            hbox->Add(serverRulesBoxSizer, wxSizerFlags().Expand().Proportion(5));
        }

        sizer->Add(hbox, wxSizerFlags().Expand().Proportion(5));
    }

    SetMinSize(wxSize(1000, 600));
}

void MyFrame::AppendServer(ListViewTab tab, const ServerInfo& info)
{
    auto& listView = m_listViews[tab];

    auto index = FindItemByData(tab, info.m_host);
    if (index == -1)
    {
        index = listView->GetItemCount();

        wxListItem item;
        item.SetId(index);
        item.SetData(new ServerHost(info.m_host));

        listView->InsertItem(item);
    }

    ServerHost* host = (ServerHost*)m_selectedServerItem.GetData();
    if (host && host->m_ip == info.m_host.m_ip && host->m_port == info.m_host.m_port)
    {
        m_playerListbox->Clear();

        for (const auto& playerInfo : info.m_players)
            m_playerListbox->Append(playerInfo.playerName);
    }

    listView->SetItem(index, (int)ListColumnID::ICON, "", !info.m_passworded ? 0 : 1);
    listView->SetItem(index, (int)ListColumnID::NAME, info.m_name);
    listView->SetItem(index, (int)ListColumnID::PING, std::to_string(info.m_ping));
    listView->SetItem(index, (int)ListColumnID::PLAYERS, std::format("{}/{}", info.m_players.size(), info.m_maxPlayers));
    listView->SetItem(index, (int)ListColumnID::VERSION, info.m_version);
    listView->SetItem(index, (int)ListColumnID::GAMEMODE, info.m_gamemode);

    m_serverRulesListView->DeleteAllItems();

    for (auto const& [key, value] : info.m_rules)
    {
        wxListItem item;
        item.SetId(1);

        m_serverRulesListView->InsertItem(item);
        m_serverRulesListView->SetItem(0, (int)ListColumnID::KEY, key.c_str());
        m_serverRulesListView->SetItem(0, (int)ListColumnID::VALUE, value.c_str());
    }
}

void MyFrame::RemoveServer(ListViewTab tab, const ServerHost& host)
{
    auto& listView = m_listViews[tab];

    wxListItem item;
    item.SetId(FindItemByData(tab, host));
    listView->GetItem(item);
    listView->DeleteItem(item);
}

void MyFrame::RemoveAllServers(ListViewTab tab)
{
    auto& listView = m_listViews[tab];

    long index = -1;
    while ((index = listView->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE)) != -1)
        listView->DeleteItem(index);
}

ListViewTab MyFrame::GetCurrentTab()
{
    return (ListViewTab)m_notebook->GetSelection();
}

long MyFrame::FindItemByData(ListViewTab tab, const ServerHost& data)
{
    auto& listView = m_listViews[tab];
    long  index    = -1;
    while ((index = listView->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE)) != -1)
    {
        wxListItem item;
        item.SetId(index);
        listView->GetItem(item);

        if (*(ServerHost*)item.GetData() == data)
            return index;
    }
    return -1;
}

void MyFrame::SetCurrentTab(ListViewTab tab)
{
    m_notebook->SetSelection((int)tab);
}

void MyFrame::AddTab(ListViewTab tab, const String& name)
{
    auto panel = new wxPanel(m_notebook);
    m_notebook->AddPage(panel, name, true);

    auto imageList = new wxImageList(16, 16);
    imageList->Add(wxIcon("IDI_UNLOCKED", wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
    imageList->Add(wxIcon("IDI_LOCKED", wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));

    auto listView = new wxListView(panel);
    listView->AssignImageList(imageList, wxIMAGE_LIST_SMALL);
    listView->InsertColumn((int)ListColumnID::ICON, "", wxLIST_FORMAT_CENTER);
    listView->InsertColumn((int)ListColumnID::NAME, "Server Name");
    listView->InsertColumn((int)ListColumnID::PING, "Ping");
    listView->InsertColumn((int)ListColumnID::PLAYERS, "Players");
    listView->InsertColumn((int)ListColumnID::VERSION, "Version");
    listView->InsertColumn((int)ListColumnID::GAMEMODE, "Gamemode");
    listView->Bind(wxEVT_LIST_ITEM_SELECTED, &MyFrame::OnItemSelected, this);
    listView->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MyFrame::OnItemActivated, this);
    listView->Bind(wxEVT_LIST_DELETE_ITEM, &MyFrame::OnDeleteItem, this);
    listView->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &MyFrame::OnRightClickItem, this);

    listView->SetColumnWidth((int)ListColumnID::ICON, 28);
    listView->SetColumnWidth((int)ListColumnID::NAME, 350);
    listView->SetColumnWidth((int)ListColumnID::GAMEMODE, 120);

    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(listView, 1, wxALL | wxEXPAND);
    panel->SetSizerAndFit(sizer);

    m_listViews.insert_or_assign(tab, listView);
}

void MyFrame::OnPageChange(wxBookCtrlEvent& event)
{
    ListViewTab curTab = (ListViewTab)event.GetSelection();

    Logger::Debug("OnPageChange: {} {}", (int)curTab, (int)event.GetSelection());

    m_serverRulesListView->DeleteAllItems();

    switch (curTab)
    {
    case ListViewTab::FAVORITES:
        Logger::Debug("Tab changed to local resource");
        for (auto& [id, info] : gBrowser->m_favoriteList)
            gBrowser->QueryServer(info);
        break;
    case ListViewTab::INTERNET:
    case ListViewTab::OFFICIAL: {
        Logger::Debug("Tab changed to internet resource");
        RemoveAllServers(curTab);

        gBrowser->m_serversList.clear();

        Logger::Debug("Current tab @ prepreprelambda: {}", (int)curTab);
        if (curTab == ListViewTab::INTERNET || curTab == ListViewTab::OFFICIAL)
        {
            Logger::Debug("Current tab @ preprelambda: {}", (int)curTab);
            Logger::Debug("Current tab @ prelambda: {}", (int)curTab);
            std::thread([curTab]() {
                Logger::Debug("Requesting master list...");
                Logger::Debug("Current tab @ lambda: {}", (int)curTab);

                String jsonResponse;
                jsonResponse.reserve(2048);

                String url = gBrowser->m_settings.masterlist;
                if (curTab == ListViewTab::INTERNET)
                    url += "/servers";
                else if (curTab == ListViewTab::OFFICIAL)
                    url += "/official";
                else
                {
                    Logger::Error("Invalid tab: {}", (int)curTab);
                    return;
                }

                Logger::Debug("Requesting master list: {}", url);

                static bool waitingRequest = false;
                if (waitingRequest)
                {
                    Logger::Warning("Requesting master list already in progress.");
                    return;
                }

                waitingRequest              = true;
                BrowserRequestResult result = gBrowser->MakeHttpRequest(url.c_str(), jsonResponse);
                waitingRequest              = false;

                Logger::Debug("Requesting master list done.");

                static const char* genericError = "Can't get information from master list.";

                if (result.code != CURLE_OK)
                {
                    Logger::Error("Can't get information from master list. Error: {}", result.errorMessage);
                    wxMessageBox("Can't get information from master list.", "Error", wxOK | wxICON_ERROR);
                }
                else if (result.httpCode != 200)
                {
                    Logger::Error("Can't get information from master list. HTTP code: {}", result.httpCode);
                    wxMessageBox("Can't get information from master list.", "Error", wxOK | wxICON_ERROR);
                }
                else
                {
                    Logger::Debug("Parsing master list...");

                    if (gBrowser->ParseMasterListResponse(jsonResponse.data()))
                    {
                        for (auto& [id, info] : gBrowser->m_serversList)
                            gBrowser->QueryServer(info);
                    }
                    else
                    {
                        wxMessageBox("Can't parse master list data.", "Error", wxOK | wxICON_ERROR);
                    }
                }
            }).detach();
        }
        break;
    }
    default:
        break;
    }
}

void MyFrame::OnItemSelected(wxListEvent& event)
{
    auto curTab = GetCurrentTab();

    m_selectedServerItem.SetId(event.GetIndex());
    m_listViews[curTab]->GetItem(m_selectedServerItem);

    ServerHost host = *(ServerHost*)m_selectedServerItem.GetData();

    auto& serverList = gBrowser->GetServerListFromTab(curTab);
    auto& serverInfo = serverList[host.ToString()];

    m_playerListbox->Clear();
    m_serverRulesListView->DeleteAllItems();

    gBrowser->QueryServer(serverInfo);
}

void MyFrame::OnItemActivated(wxListEvent& event)
{
    auto curTab = GetCurrentTab();

    m_selectedServerItem.SetId(event.GetIndex());
    m_listViews[curTab]->GetItem(m_selectedServerItem);

    if (gBrowser->m_settings.nickname.empty())
    {
        wxMessageBox("You haven't set up a nickname", "Error", wxOK | wxICON_ERROR);

        wxCommandEvent evt(wxEVT_MENU, ID_SETTINGS);
        wxPostEvent(this, evt);
        return;
    }

    ServerHost host = *(ServerHost*)m_selectedServerItem.GetData();

    if (!gBrowser->LaunchGame(host.m_ip, host.m_port))
        wxMessageBox("Failed to launch the game. Is it the path correct?", "Error", wxOK | wxICON_ERROR);
}

void MyFrame::OnDeleteItem(wxListEvent& event)
{
    wxListItem item;
    item.SetId(event.GetIndex());
    m_listViews[GetCurrentTab()]->GetItem(item);

    delete (ServerHost*)item.GetData();
}

void MyFrame::OnRightClickItem(wxListEvent& event)
{
    auto menu = std::make_unique<wxMenu>();
    menu->Append(wxID_COPY, "Copy Server IP:Port");

    if (GetCurrentTab() == ListViewTab::FAVORITES)
    {
        menu->Append(wxID_DELETE, "Delete Server");
    }
    else
    {
        menu->Append(wxID_ADD, "Add to Favorites");
    }

    menu->Bind(
        wxEVT_MENU, [&](wxCommandEvent& cmdEvent) -> void {
        auto host = *(ServerHost*)m_listViews[GetCurrentTab()]->GetItemData(event.GetIndex());

        if (wxTheClipboard->Open())
        {
            /* docs: "After this function has been called, the clipboard owns the data, so do not delete the data explicitly." */
            wxTheClipboard->SetData(new wxTextDataObject(host.ToString()));
            wxTheClipboard->Close();
        }
    },
        wxID_COPY);

    menu->Bind(
        wxEVT_MENU, [&](wxCommandEvent& cmdEvent) -> void {
        auto host = *(ServerHost*)m_listViews[GetCurrentTab()]->GetItemData(event.GetIndex());
        gBrowser->RemoveFromFavorites(host);
        gBrowser->SaveSettings();
    },
        wxID_DELETE);

    menu->Bind(
        wxEVT_MENU, [&](wxCommandEvent& cmdEvent) -> void {
        auto host = *(ServerHost*)m_listViews[GetCurrentTab()]->GetItemData(event.GetIndex());
        gBrowser->AddToFavorites(host);
        gBrowser->SaveSettings();
    },
        wxID_ADD);

    PopupMenu(menu.get(), event.GetPoint());
}

void MyFrame::OnSettings(wxCommandEvent& event)
{
    SettingsDialog dialog("Settings");
    dialog.ShowModal();
}

void MyFrame::OnAddServer(wxCommandEvent& event)
{
    AddServerDialog dialog("Add Server");
    dialog.ShowModal();
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    AboutDialog dialog("About");
    dialog.ShowModal();
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
