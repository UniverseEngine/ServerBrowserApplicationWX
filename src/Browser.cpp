#include "Browser.hpp"
#include "MyFrame.hpp"

#include <Launcher.hpp>
#include <wx/msw/private/hiddenwin.h>

#include <nlohmann/json.hpp>

#include <thread>

#include <cpr/cpr.h>

#include <Core/UniverseLogger.hpp>

#include <format>
#include <fstream>
#include <stack>

using json = nlohmann::json;

std::unique_ptr<Browser>     gBrowser;
std::unique_ptr<std::thread> gRequestThread;

using namespace Universe;

using ErrorCallback    = std::function<void(const std::string&)>;
using ResponseCallback = std::function<void(const cpr::Response&, ErrorCallback)>;

struct RequestData
{
    std::string      url;
    ResponseCallback callback;
    ErrorCallback    errorCallback;
};

class RequestQueue {
private:
    std::unique_ptr<std::thread>  m_thread;
    std::mutex                    m_mutex;
    std::queue<RequestData>       m_queue;
    std::unique_ptr<cpr::Session> m_session;
    std::mutex                    m_sessionMutex;

public:
    RequestQueue()
    {
        m_session = std::make_unique<cpr::Session>();
        m_session->SetHeader(cpr::Header { { "Content-Type", "application/json" } });
        m_session->SetTimeout(cpr::Timeout { 3000 });

        m_thread = std::make_unique<std::thread>(&RequestQueue::ProcessQueue, this);
    }

    void AddRequest(const RequestData& request)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(request);
    }

    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    RequestData GetNextRequest()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        request = m_queue.front();
        m_queue.pop();
        return request;
    }

    void ProcessQueue()
    {
        while (true)
        {
            if (!IsEmpty())
            {
                auto requestData = GetNextRequest();
                auto response    = PerformGet(requestData);
                requestData.callback(response, requestData.errorCallback);
            }
        }
    }

    cpr::Response PerformGet(const RequestData& request)
    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        m_session->SetUrl(cpr::Url { request.url });
        auto response = m_session->Get();
        return response;
    };
};

std::unique_ptr<RequestQueue> gRequestQueue = std::make_unique<RequestQueue>();

LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void Browser::QueryServer(std::shared_ptr<ServerInfo> serverInfo)
{
    // Return if the server info is invalid
    if (serverInfo->m_host.m_ip.empty() || serverInfo->m_host.m_port == 0)
    {
        Logger::Error("How the hell do we even get here? IP or port is empty.");
        return;
    }

    auto url = std::format("http://{}:{}/server", serverInfo->m_host.m_ip, serverInfo->m_host.m_port);

    auto error = [&](const std::string& message) {
        Logger::Error(message);
        serverInfo->SetAsOffline();
    };

    auto handleResponse = [&](const cpr::Response& response, std::function<void(const std::string&)> onError) {
        if (response.error)
        {
            onError(std::format("Failed to query server: {}", response.error.message));
            return;
        }

        try
        {
            json data = json::parse(response.text);

            serverInfo->m_name       = data["name"];
            serverInfo->m_maxPlayers = data["max_players"];
            serverInfo->m_passworded = data["passworded"];
            serverInfo->m_gamemode   = data["gamemode"];
            serverInfo->m_version    = data["version"];

            serverInfo->m_players.clear();
            std::for_each(data["players"].begin(), data["players"].end(), [&](const auto& player) {
                serverInfo->m_players.push_back(player.get<std::string>());
            });

            serverInfo->m_rules.clear();
            std::for_each(data["rules"].items().begin(), data["rules"].items().end(), [&](const auto& elems) {
                serverInfo->m_rules.insert_or_assign(std::string { elems.key() }, std::string { elems.value() });
            });

            serverInfo->m_ping   = response.elapsed;
            serverInfo->m_online = true;

            m_frame->AppendServer(m_frame->GetCurrentTab(), serverInfo.get());
        }
        catch (std::exception& ex)
        {
            onError(std::format("Failed to parse server response: {}", ex.what()));
        }
    };

    gRequestQueue->AddRequest({ url, handleResponse, error });
}

void Browser::LaunchGame(const std::string& host, uint16_t port)
{
    Launcher::LaunchData data = {};
    {
        auto game = &data.game;

        auto generateArguments = [&]() -> Launcher::Arguments {
            Launcher::Arguments args;

            args.Append("ip", host);
            args.Append("port", std::to_string(port));
            args.Append("nickname", m_settings.nickname);
            if (m_settings.windowed)
            {
                args.Append("windowed");
            }

            return args;
        };

        game->path      = m_settings.gamePath;
        game->title     = "III";
        game->arguments = generateArguments();

        std::filesystem::path curPath = std::filesystem::current_path();

        data.mods.push_back({ curPath / "IIIGameModule.dll", "IIIGameModule" });

        if (m_settings.showConsole)
        {
            data.mods.push_back({ curPath / "ConsoleModule.dll", "ConsoleModule" });
        }

        SetDllDirectory(curPath.wstring().c_str());
    }

    Launcher::LauncherSystem launcher;
    Launcher::LaunchResult   result = launcher.Launch(data);

    if (result.first != Launcher::LaunchResultCode::Success)
    {
        std::string error_message = std::format("Failed to launch the game:\n\n{}", result.second);
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
    for (auto& [id, info] : m_serversList[ServerListType::FAVORITES])
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

    std::string contents;
    contents.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

    if (contents.length() == 0)
        goto close;

    try
    {
        json data = json::parse(contents);

        m_settings.nickname    = (!data["nickname"].is_string()) ? "" : data["nickname"].get<std::string>();
        m_settings.gamePath    = (!data["gamePath"].is_string()) ? "" : data["gamePath"].get<std::string>();
        m_settings.proxy       = (!data["proxy"].is_string()) ? "" : data["proxy"].get<std::string>();
        m_settings.masterlist  = (!data["masterlist"].is_string()) ? gDefaultMasterlistUrl : data["masterlist"].get<std::string>();
        m_settings.windowed    = (!data["windowed"].is_boolean()) ? false : data["windowed"].get<bool>();
        m_settings.showConsole = (!data["showConsole"].is_boolean()) ? false : data["showConsole"].get<bool>();

        for (auto& element : data["favorites"])
        {
            if (!element["ip"].is_string() || !element["port"].is_number())
                continue; // invalid entry, skip

            std::string ip   = element["ip"].get<std::string>();
            uint16_t    port = element["port"].get<uint16_t>();

            AddToFavorites(ServerHost(element["ip"].get<std::string>(), element["port"].get<uint16_t>()));
        }
    }
    catch (json::parse_error& ex)
    {
        wxMessageBox("Failed to parse settings file", "Error", wxOK | wxICON_ERROR);
    }
    catch (std::exception& ex)
    {
        wxMessageBox(ex.what(), "Error", wxOK | wxICON_ERROR);
    }

close:
    stream.close();
}

void Browser::AddToFavorites(const ServerHost& host)
{
    m_serversList[ServerListType::FAVORITES].insert_or_assign(host.ToString(), std::make_shared<ServerInfo>(host.m_ip, host.m_port));

    auto& info = m_serversList[ServerListType::FAVORITES][host.ToString()];

    m_frame->AppendServer(ServerListType::FAVORITES, info.get());

    QueryServer(info);
}

void Browser::RemoveFromFavorites(const ServerHost& host)
{
    m_frame->RemoveServer(ServerListType::FAVORITES, host);

    m_serversList[ServerListType::FAVORITES].erase(host.ToString());
}

void Browser::GetServersFromMasterlist(ServerListType type)
{
    std::string url = m_settings.masterlist;

    if (type == ServerListType::OFFICIAL)
    {
        url += "/official";
    }
    else
    {
        url += "/servers";
    }

    std::string data;

    auto handleResponse = [&](const cpr::Response& response, ErrorCallback onError) {
        if (response.error)
        {
            onError(std::format("Failed to request masterlist: {}", response.error.message));
            return;
        }

        if (response.status_code != 200)
        {
            onError(std::format("Can't get information from master list. HTTP code: {}", response.status_code));
            return;
        }

        try
        {
            json jsonData = json::parse(response.text);

            if (jsonData.is_array() && jsonData.empty())
            {
                Logger::Debug("Masterlist is empty");
                return;
            }

            auto& serverList = m_serversList[type];

            for (auto& element : jsonData)
            {
                auto serverInfo { std::make_shared<ServerInfo>(element["ip"], element["port"]) };

                serverList.insert_or_assign(serverInfo->m_host.ToString(), serverInfo);
            }

            for (auto& [id, info] : serverList)
            {
                QueryServer(info);
            }
        }
        catch (json::parse_error& ex)
        {
            onError(std::format("Failed to parse masterlist response: {}", ex.what()));
        }
    };

    gRequestQueue->AddRequest({ url, handleResponse, [&](const std::string& message) {
        Logger::Error(message);
        wxMessageBox(message.c_str(), "Error", wxOK | wxICON_ERROR);
    } });


}
