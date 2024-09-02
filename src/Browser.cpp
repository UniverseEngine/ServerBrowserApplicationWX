
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

void Browser::QueryServer(std::shared_ptr<ServerInfo> serverInfo)
{
    std::thread([=]() {
        if (!serverInfo)
            return;

        std::string url = std::format("http://{}:{}/server", serverInfo->m_host.m_ip, serverInfo->m_host.m_port);
        std::string jsonStr;

        const std::chrono::time_point<std::chrono::system_clock> tp_now = std::chrono::system_clock::now();

        auto result = MakeHttpRequest(url, jsonStr);

        if (result.code != CURLE_OK)
        {
            serverInfo->m_online = false;
            serverInfo->m_ping   = 9999;
            return;
        }

        try
        {
            json data = json::parse(jsonStr);

            serverInfo->m_name       = data["name"];
            serverInfo->m_maxPlayers = data["max_players"];
            serverInfo->m_passworded = data["passworded"];
            serverInfo->m_gamemode   = data["gamemode"];
            serverInfo->m_version    = data["version"];

            serverInfo->m_players.clear();
            for (int i = 0; i < data["players"].size(); i++)
            {
                serverInfo->m_players.push_back({ data["players"][i]["name"] });
            }

            serverInfo->m_rules.clear();
            for (auto& elems : data["rules"].items())
            {
                serverInfo->m_rules.insert_or_assign(std::string { elems.key() }, std::string { elems.value() });
            }

            serverInfo->m_ping   = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tp_now).count();
            serverInfo->m_online = true;

            m_frame->AppendServer(m_frame->GetCurrentTab(), serverInfo.get());
        }
        catch (std::exception& ex)
        {
            Logger::Error("Failed to parse server response: {}", ex.what());

            serverInfo->m_online = false;
            serverInfo->m_ping   = 9999;
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

void Browser::LaunchGame(const String& host, uint16_t port)
{
    Launcher::LaunchData data = {};

    data.game.path  = m_settings.gamePath;
    data.game.title = "III";
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
    Launcher::LaunchResult   result = launcher.Launch(data);

    if (result.first != Launcher::LaunchResultCode::Success)
    {
        String error_message = fmt::format("Failed to launch the game:\n\n{}", result.second);
        wxMessageBox(error_message.c_str(), "Error", wxOK | wxICON_ERROR);
    }
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
    data["gamePath"]    = m_settings.gamePath;
    data["proxy"]       = m_settings.proxy;
    data["masterlist"]  = m_settings.masterlist;
    data["windowed"]    = m_settings.windowed;
    data["showConsole"] = m_settings.showConsole;

    /* favorites */
    for (auto& [id, info] : m_serversList[ListViewTab::FAVORITES])
        data["favorites"].push_back(
            { { "ip", info->m_host.m_ip },
                { "port", info->m_host.m_port } });

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
        m_settings.gamePath    = (!data["gamePath"].is_string()) ? "" : data["gamePath"].get<String>();
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
    m_serversList[ListViewTab::FAVORITES].insert_or_assign(host.ToString(), std::make_shared<ServerInfo>(host.m_ip, host.m_port));

    auto& info = m_serversList[ListViewTab::FAVORITES][host.ToString()];

    m_frame->AppendServer(ListViewTab::FAVORITES, info.get());

    QueryServer(info);
}

void Browser::RemoveFromFavorites(const ServerHost& host)
{
    m_frame->RemoveServer(ListViewTab::FAVORITES, host);

    m_serversList[ListViewTab::FAVORITES].erase(host.ToString());
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
        return;
    }
    else if (result.httpCode != 200)
    {
        Logger::Error("Can't get information from master list. HTTP code: {}", result.httpCode);
        wxMessageBox("Can't get information from master list.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    auto& serverList = type == MasterListRequestType::ALL_SERVERS ? m_serversList[ListViewTab::INTERNET] : m_serversList[ListViewTab::OFFICIAL];
    try
    {
        json jsonData = json::parse(data);

        for (auto& element : jsonData)
        {
            auto serverInfo { std::make_shared<ServerInfo>(element["ip"], element["port"]) };

            serverList.insert_or_assign(serverInfo->m_host.ToString(), serverInfo);
        }
    }
    catch (json::parse_error& ex)
    {
        Logger::Error("Failed to parse masterlist response: {}", ex.what());
        wxMessageBox("Can't parse master list data.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    for (auto& [id, info] : serverList)
        QueryServer(info);
}
