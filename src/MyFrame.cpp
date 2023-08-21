
#include "pch.hpp"

#include <wx/clipbrd.h>

#include "dialogs/AboutDialog.hpp"
#include "dialogs/AddServerDialog.hpp"
#include "dialogs/SettingsDialog.hpp"

#include "MyFrame.hpp"

#include "Browser.hpp"

enum
{
    ID_ADDSERVER = 1,
    ID_SETTINGS
};

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Server Browser", wxDefaultPosition, wxSize(1000, 600),
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
            m_notebook = new wxNotebook(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(800, 600));

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

            m_listbox = new wxListBox(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(160, 570));

            playerBoxSizer->Add(m_listbox, 1, wxALL | wxEXPAND, 5);

            hbox->Add(playerBoxSizer);
        }

        sizer->Add(hbox, 1);
    }
    /*
    {
        auto hbox = new wxBoxSizer(wxHORIZONTAL);
        {
            auto serverInfoBoxSizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel, "Server Info");

            wxFlexGridSizer* fgs = new wxFlexGridSizer(2, 7, 25);
            {
                fgs->Add(new wxStaticText(mainPanel, wxID_ANY, "Server Name: ", wxDefaultPosition, wxDefaultSize));
                fgs->Add(m_serverInfoName = new wxStaticText(mainPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize));

                fgs->Add(new wxStaticText(mainPanel, wxID_ANY, "Server IP: ", wxDefaultPosition, wxDefaultSize));
                fgs->Add(m_serverInfoHost = new wxStaticText(mainPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize));

                fgs->Add(new wxStaticText(mainPanel, wxID_ANY, "Server Players: ", wxDefaultPosition, wxDefaultSize));
                fgs->Add(m_serverInfoPlayers = new wxStaticText(mainPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize));

                fgs->Add(new wxStaticText(mainPanel, wxID_ANY, "Server Ping: ", wxDefaultPosition, wxDefaultSize));
                fgs->Add(m_serverInfoPing = new wxStaticText(mainPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize));
            }

            serverInfoBoxSizer->Add(fgs, 1, wxALL | wxEXPAND, 5);

            hbox->Add(serverInfoBoxSizer, wxSizerFlags().Expand().Proportion(5));
        }

        sizer->Add(hbox, wxSizerFlags().Expand().Proportion(5));
    }
    */

    SetMinSize(wxSize(1000, 600));
}

void MyFrame::AppendServer(ListViewTab tab, const ServerInfo& info)
{
    AppendServer(tab, info.m_host, info.m_passworded, info.m_name, info.m_ping, info.m_players, info.m_maxPlayers, info.m_version, info.m_gamemode);
}

void MyFrame::AppendServer(ListViewTab tab, const ServerHost& host, bool locked, const String& name, uint32_t ping, uint32_t players, uint32_t maxPlayers, const String& version, const String& gamemode, const String& lastPlayed)
{
    auto& listView = m_listViews[tab];

    auto index = FindItemByData(tab, host);
    if (index == -1)
    {
        index = listView->GetItemCount();

        wxListItem item;
        item.SetId(index);
        item.SetData(new ServerHost(host));

        listView->InsertItem(item);
    }

    listView->SetItem(index, (int)ListColumnID::ICON, "", !locked ? 0 : 1);
    listView->SetItem(index, (int)ListColumnID::NAME, name);
    listView->SetItem(index, (int)ListColumnID::PING, std::to_string(ping));
    listView->SetItem(index, (int)ListColumnID::PLAYERS, std::format("{}/{}", players, maxPlayers));
    listView->SetItem(index, (int)ListColumnID::VERSION, version);
    listView->SetItem(index, (int)ListColumnID::GAMEMODE, gamemode);
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

void MyFrame::AppendPlayer(const String& name)
{
    m_listbox->Append(name);
}

void MyFrame::RemoveAllPlayers()
{
    m_listbox->Clear();
}

ListViewTab MyFrame::GetCurrentTab()
{
    return (ListViewTab)m_notebook->GetSelection();
}

long MyFrame::FindItemByData(ListViewTab tab, const ServerHost& data)
{
    auto& listView = m_listViews[tab];
    long index = -1;
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
    
    switch (curTab)
    {
    case ListViewTab::FAVORITES:
        for (auto& [id, info] : gBrowser->m_favoriteList)
            info.Query(gBrowser->m_socket);
        break;
    case ListViewTab::INTERNET:
    case ListViewTab::OFFICIAL: {
        RemoveAllServers(curTab);

        gBrowser->m_serversList.clear();

        if (curTab == ListViewTab::INTERNET || curTab == ListViewTab::OFFICIAL)
        {
            static bool waitingRequest = false;
            if (waitingRequest)
                break;

            waitingRequest = true;

            std::thread([&]() {
                String jsonResponse;
                jsonResponse.reserve(2048);

                String url;
                if (curTab == ListViewTab::INTERNET)
                    url = gBrowser->m_settings.masterlist + "/servers";
                else if (curTab == ListViewTab::OFFICIAL)
                    url = gBrowser->m_settings.masterlist + "/official";

                CURLcode code;
                char     errbuf[CURL_ERROR_SIZE] = { 0 };

                curl_easy_setopt(gBrowser->m_curl, CURLOPT_ERRORBUFFER, errbuf);

                if ((code = gBrowser->Request(url.c_str(), jsonResponse)) != CURLE_OK)
                {
                    std::size_t len      = strlen(errbuf);
                    String      strerror = len ? errbuf : curl_easy_strerror(code);

                    wxMessageBox(std::format("Can't get information from master list.\nError: {}", strerror), "Error", wxOK | wxICON_ERROR);
                }
                else
                {
                    if (gBrowser->ParseMasterListResponse(jsonResponse.data()))
                    {
                        for (auto& [id, info] : gBrowser->m_serversList)
                            info.Query(gBrowser->m_socket);
                    }
                    else
                        wxMessageBox("Can't parse master list data.", "Error", wxOK | wxICON_ERROR);
                }

                waitingRequest = false;
            })
                .detach();
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

    wxListItem item;
    item.SetId(event.GetIndex());
    m_listViews[curTab]->GetItem(item);

    ServerHost host = *(ServerHost*)item.GetData();

    auto& serverList = gBrowser->GetServerListFromTab(curTab);
    serverList[host.ToString()].Query(gBrowser->m_socket);

    RemoveAllPlayers();

    for (auto& name : serverList[host.ToString()].m_playerList)
        AppendPlayer(name);
}

void MyFrame::OnItemActivated(wxListEvent& event)
{
    auto curTab = GetCurrentTab();

    wxListItem item;
    item.SetId(event.GetIndex());
    m_listViews[curTab]->GetItem(item);

    ServerHost host = *(ServerHost*)item.GetData();

    gBrowser->LaunchGame(host.m_ip, host.m_port);
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