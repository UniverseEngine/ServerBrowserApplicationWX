
#include "Browser.hpp"
#include "MyFrame.hpp"

#include <Launcher.hpp>
#include <wx/msw/private/hiddenwin.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

Unique<Browser> gBrowser;

LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SOCKET: {
        if (WSAGETSELECTEVENT(lParam) == FD_READ)
            gBrowser->ReadFromSocket();

        auto
            server_count       = 0,
            player_count       = 0,
            serverplayer_count = 0;
        for (auto& [id, info] : gBrowser->m_serversList)
        {
            if (!info.m_online)
                continue;

            server_count++;
            player_count += info.m_players;
        }

        gBrowser->m_frame->SetStatusText(std::format(L"Servers: {} players, playing in {} servers", player_count, server_count));
        break;
    }
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

Browser::Browser(MyFrame* frame)
    : m_frame(frame)
    , m_curl(nullptr)
{
    m_curl = curl_easy_init();

    {
        LPCTSTR pclassname = nullptr;

        m_hwnd = wxCreateHiddenWindow(&pclassname, TEXT("_wxSocket_Internal_Window_Class"), wndProc);
        if (!m_hwnd)
            abort();
    }

    /* socket */
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NOERROR)
            abort();

        m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_socket == INVALID_SOCKET)
            abort();

        uint32_t timeout = 2000;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

        struct sockaddr_in bindaddr = { AF_INET };
        if (bind(m_socket, (sockaddr*)&bindaddr, 16) != NO_ERROR)
            abort();

        if (WSAAsyncSelect(m_socket, m_hwnd, WM_SOCKET, FD_READ) == SOCKET_ERROR)
            abort();
    }
}

Browser::~Browser()
{
    if (m_curl)
        curl_easy_cleanup(m_curl);

    WSACleanup();
}

Browser::ServerMap& Browser::GetServerListFromTab(ListViewTab tab)
{
    switch (tab)
    {
    default:
    case ListViewTab::FAVORITES:
        return m_favoriteList;
    case ListViewTab::INTERNET:
    case ListViewTab::OFFICIAL:
    case ListViewTab::LAN:
        return m_serversList;
    }
}

void Browser::QueryServer(ServerInfo& serverInfo)
{
    serverInfo.m_lastPingRecv = Utils::GetTickCount();

    struct sockaddr_in sendaddr = { AF_INET };
    sendaddr.sin_addr.s_addr    = inet_addr(serverInfo.m_host.m_ip.c_str());
    sendaddr.sin_port           = htons(serverInfo.m_host.m_port);

    char buffer[] = { 'Q', 'U', 'E', 'R', 'Y', 's' };
    sendto(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&sendaddr, sizeof(sendaddr));

    char buffer2[] = { 'Q', 'U', 'E', 'R', 'Y', 'p' };
    sendto(m_socket, buffer2, sizeof(buffer2), 0, (sockaddr*)&sendaddr, sizeof(sendaddr));
}

void Browser::ReadFromSocket()
{
    char buffer[2048];

    struct sockaddr_in recvAddr;
    int                recvAddrSize = sizeof(recvAddr);
    int                dataLength   = recvfrom(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&recvAddr, &recvAddrSize);

    if (dataLength == SOCKET_ERROR)
        return;

    String   ip   = inet_ntoa(recvAddr.sin_addr);
    uint16_t port = ntohs(recvAddr.sin_port);

    auto  curTab = m_frame->GetCurrentTab();
    auto& list   = GetServerListFromTab(curTab);

    char type = buffer[0];
    switch (type)
    {
    case 's': {
        const char* ptr = buffer + sizeof(char);

        String version = ptr;
        ptr += strlen(ptr) + 1;

        String name = ptr;
        ptr += strlen(ptr) + 1;

        String game_mode = ptr;
        ptr += strlen(ptr) + 1;

        bool passworded = (char)*ptr;
        ptr += sizeof(char);

        uint8_t players = (uint8_t)*ptr;
        ptr += sizeof(uint8_t);

        uint8_t max_players = (uint8_t)*ptr;
        ptr += sizeof(uint8_t);

        for (auto& [id, info] : list)
        {
            if (info.m_host.m_ip != ip || info.m_host.m_port != port)
                continue;

            info.m_version    = version;
            info.m_name       = name;
            info.m_players    = players;
            info.m_maxPlayers = max_players;
            info.m_passworded = passworded;
            info.m_gamemode   = game_mode;

            info.m_ping   = Utils::GetTickCount() - info.m_lastPingRecv;
            info.m_online = true;

            m_frame->AppendServer(m_frame->GetCurrentTab(), info);

            break;
        }
        break;
    }
    case 'p': {
        const char* ptr = buffer + sizeof(char);
        const char* end = buffer + dataLength - sizeof(char);
        for (auto& [id, info] : list)
        {
            if (info.m_host.m_ip != ip || info.m_host.m_port != port)
                continue;

            info.m_playerList.clear();
            while (ptr < end)
            {
                info.m_playerList.emplace_back(ptr);
                ptr += strlen(ptr) + 1;
            }
        }

        break;
    }
    }
}

CURLcode Browser::Request(String url, String& data)
{
    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_PROXY, m_settings.proxy.empty() ? nullptr : m_settings.proxy.c_str());
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(m_curl, CURLOPT_CAINFO, "cacert.pem");
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(
        m_curl, CURLOPT_WRITEFUNCTION, +[](char* buffer, size_t size, size_t nitems, void* outstream) {
            size_t realsize = size * nitems;
            ((String*)outstream)->append(buffer, realsize);
            return realsize;
        });

    return curl_easy_perform(m_curl);
}

bool Browser::ParseMasterListResponse(String jsonStr)
{
    try
    {
        json data = json::parse(jsonStr);

        for (auto& element : data)
        {
            ServerInfo info(element["ip"], element["port"]);

            info.m_name       = element["name"];
            info.m_players    = element["players"];
            info.m_maxPlayers = element["max_players"];
            info.m_passworded = element["passworded"];
            info.m_gamemode   = element["gamemode"];
            info.m_version    = element["version"];
            info.m_playerList = element["player_list"];

            m_serversList.insert_or_assign(info.m_host.ToString(), info);
        }

        return true;
    }
    catch (json::parse_error& ex)
    {
    }
    return false;
}

bool Browser::LaunchGame(String host, uint16_t port)
{
    Launcher::LaunchData data = {};

    data.game.path  = m_settings.gamePath;
    data.game.title = "III";
    data.game.arguments.Append("island_loading", "high");
    data.game.arguments.Append("ip", host);
    data.game.arguments.Append("port", std::to_string(port));
    data.game.arguments.Append("nickname", m_settings.nickname);

    if (m_settings.windowed)
        data.game.arguments += "-windowed";

    Path curPath = std::filesystem::current_path();

    data.mods.push_back({ curPath / "IIIGameModule.dll", "IIIGameModule" });

    if (m_settings.showConsole)
        data.mods.push_back({ curPath / "ConsoleModule.dll", "ConsoleModule" });

    SetDllDirectory(curPath.wstring().c_str());

    Launcher::LauncherSystem launcher;
    if (!launcher.Launch(data))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void Browser::SaveSettings()
{
    std::ofstream stream;
    stream.open("browser.json");
    if (!stream.is_open())
        return;

    json data;

    /* settings */
    data["nickname"]    = m_settings.nickname;
    data["gamePath"]    = Utils::Win32::ToString(m_settings.gamePath);
    data["proxy"]       = m_settings.proxy;
    data["masterlist"]  = m_settings.masterlist;
    data["windowed"]    = m_settings.windowed;
    data["showConsole"] = m_settings.showConsole;

    /* favorites */
    for (auto& [id, info] : m_favoriteList)
        data["favorites"].push_back(
            { { "ip", info.m_host.m_ip },
                { "port", info.m_host.m_port } });

    stream << data;

    stream.close();
}

void Browser::LoadSettings()
{
    std::ifstream stream;
    stream.open("browser.json");
    if (!stream.is_open())
        return;

    String contents;
    contents.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

    if (contents.length() == 0)
        goto close;

    try
    {
        json data = json::parse(contents);

        m_settings.nickname    = (!data["nickname"].is_string()) ? "" : data["nickname"].get<String>();
        m_settings.gamePath    = (!data["gamePath"].is_string()) ? L"" : Utils::Win32::ToWideString(data["gamePath"].get<String>());
        m_settings.proxy       = (!data["proxy"].is_string()) ? "" : data["proxy"].get<String>();
        m_settings.masterlist  = (!data["masterlist"].is_string()) ? DEFAULT_MASTERLIST : data["masterlist"].get<String>();
        m_settings.windowed    = (!data["windowed"].is_boolean()) ? false : data["windowed"].get<bool>();
        m_settings.showConsole = (!data["showConsole"].is_boolean()) ? false : data["showConsole"].get<bool>();

        for (auto& element : data["favorites"])
        {
            if(!element["ip"].is_string() || !element["port"].is_number())
                continue; // invalid entry, skip

            String ip     = element["ip"].get<String>();
            uint16_t port = element["port"].get<uint16_t>();
            
            AddToFavorites(ServerHost(element["ip"].get<String>(), element["port"].get<uint16_t>()));
        }
    }
    catch (json::parse_error& ex)
    {
        wxMessageBox("Failed to parse settings file", "Error", wxOK | wxICON_ERROR);
    }
    catch (Exception& ex)
    {
        wxMessageBox(ex.what(), "Error", wxOK | wxICON_ERROR);
    }

close:
    stream.close();
}

void Browser::AddToFavorites(const ServerHost& host)
{
    ServerInfo info(host.m_ip, host.m_port);
    m_favoriteList.insert_or_assign(host.ToString(), info);

    m_frame->AppendServer(ListViewTab::FAVORITES, info);

    QueryServer(info);
}

void Browser::RemoveFromFavorites(const ServerHost& host)
{
    m_frame->RemoveServer(ListViewTab::FAVORITES, host);

    m_favoriteList.erase(host.ToString());
}