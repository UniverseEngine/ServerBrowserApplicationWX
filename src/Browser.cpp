
#include "Browser.hpp"
#include "MyFrame.hpp"

#include <Launcher.hpp>
#include <wx/msw/private/hiddenwin.h>

#include <nlohmann/json.hpp>

#include <thread>

using json = nlohmann::json;

Unique<Browser> gBrowser;

LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

Browser::Browser(MyFrame* frame)
    : m_frame(frame)
{
}

Browser::~Browser()
{
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

void Browser::QueryServer(ServerInfo& serverInfo, bool updatePlayerList)
{
    std::thread([&]() {
        std::string url = std::format("http://{}:{}/server", serverInfo.m_host.m_ip, serverInfo.m_host.m_port);
        std::string jsonStr;

        const std::chrono::time_point<std::chrono::system_clock> tp_now = std::chrono::system_clock::now();

        auto result = MakeHttpRequest(url, jsonStr);

        if (result.code != CURLE_OK)
        {
            serverInfo.m_online = false;
            serverInfo.m_ping   = 9999;
            return;
        }

        try
        {
            json data = json::parse(jsonStr);

            serverInfo.m_name       = data["name"];
            serverInfo.m_maxPlayers = data["max_players"];
            serverInfo.m_passworded = data["passworded"];
            serverInfo.m_gamemode   = data["gamemode"];
            serverInfo.m_version    = data["version"];

            serverInfo.m_players.clear();
            for (int i = 0; i < data["players"].size(); i++)
            {
                serverInfo.m_players.push_back({ data["players"][i]["name"] });
            }

            serverInfo.m_rules.clear();
            for (auto& elems : data["rules"].items())
            {
                serverInfo.m_rules.insert_or_assign(std::string { elems.key() }, std::string { elems.value() });
            }

            serverInfo.m_ping   = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tp_now).count();
            serverInfo.m_online = true;

            m_frame->AppendServer(m_frame->GetCurrentTab(), serverInfo);
        }
        catch (std::exception& ex)
        {
            Logger::Error("Failed to parse server response: {}", ex.what());

            serverInfo.m_online = false;
            serverInfo.m_ping   = 9999;
        }
    }).detach();
}

BrowserRequestResult Browser::MakeHttpRequest(const String& url, String& data) const
{
    CURL*                curl;
    BrowserRequestResult result;
    char                 errbuf[CURL_ERROR_SIZE] = { 0 };

    Logger::Debug("Requesting {}", url);

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, false);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    // clang-format off
    curl_easy_setopt(curl, CURLOPT_PROXY, m_settings.proxy.empty() ? nullptr : m_settings.proxy.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char* buffer, size_t size, size_t nitems, void* outstream) {
        size_t realsize = size * nitems;
        ((String*)outstream)->append(buffer, realsize);
        return realsize;
    });
    // clang-format on

    Logger::Debug("Performing request");
    result.code = curl_easy_perform(curl);
    if (result.code != CURLE_OK)
    {
        result.errorMessage = curl_easy_strerror(result.code);
        Logger::Error("Request failed: {}", result.errorMessage);
    }
    else
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.httpCode);
        Logger::Debug("Request finished with code {}", result.httpCode);
    }

    curl_easy_setopt(curl, CURLOPT_PROXY, nullptr);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, nullptr);
    curl_easy_setopt(curl, CURLOPT_URL, nullptr);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_cleanup(curl);

    return result;
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
            info.m_maxPlayers = element["max_players"];
            info.m_passworded = element["passworded"];
            info.m_gamemode   = element["gamemode"];
            info.m_version    = element["version"];

            for (int i = 0; i < data["players"].size(); i++)
            {
                info.m_players.push_back({ data["players"][i]["name"] });
            }

            m_serversList.insert_or_assign(info.m_host.ToString(), info);
        }

        return true;
    }
    catch (json::parse_error& ex)
    {
        Logger::Error("Failed to parse masterlist response: {}", ex.what());
    }
    return false;
}

bool Browser::LaunchGame(const String& host, uint16_t port)
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
        return false;
    return true;
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
            if (!element["ip"].is_string() || !element["port"].is_number())
                continue; // invalid entry, skip

            String   ip   = element["ip"].get<String>();
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
    m_favoriteList.insert_or_assign(host.ToString(), ServerInfo(host.m_ip, host.m_port));

    auto& info = m_favoriteList[host.ToString()];

    m_frame->AppendServer(ListViewTab::FAVORITES, info);

    QueryServer(info);
}

void Browser::RemoveFromFavorites(const ServerHost& host)
{
    m_frame->RemoveServer(ListViewTab::FAVORITES, host);

    m_favoriteList.erase(host.ToString());
}

void Browser::RequestMasterList(MasterListRequestType type)
{
    String url = m_settings.masterlist;

    switch (type)
    {
    case MasterListRequestType::ALL_SERVERS:
        url += "/servers";
        break;
    case MasterListRequestType::OFFICIAL_SERVERS:
        url += "/official";
        break;
    }

    String data;

    auto result = MakeHttpRequest(url, data);
    if (result.code != CURLE_OK)
    {
        String   errorMessage = std::format("Failed to request masterlist: {}", result.errorMessage);
        wxString wxErrorMessage(errorMessage.c_str(), wxConvUTF8);

        Logger::Error(errorMessage);
        wxMessageBox(wxErrorMessage, "Error", wxOK | wxICON_ERROR);
    }

    if (!ParseMasterListResponse(data))
        return;

    for (auto& [id, info] : m_serversList)
        QueryServer(info);
}
