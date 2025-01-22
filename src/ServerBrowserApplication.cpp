#include "ServerBrowserApplication.hpp"

#include "MyFrame.hpp"
#include "Browser.hpp"

#include <filesystem>

#include <Core/UniverseLogger.hpp>

bool ServerBrowserApplication::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);

    wchar_t _path[MAX_PATH];
    GetModuleFileName(NULL, _path, MAX_PATH);
    std::filesystem::current_path(std::filesystem::path(_path).remove_filename());
    std::filesystem::create_directories(std::filesystem::current_path() / "logs");

    Universe::Logger::Initialise("logs/browser.log");

    // Connect console stdout if available
    if (AllocConsole())
    {
        FILE* file;
        freopen_s(&file, "CONOUT$", "w", stdout);
        freopen_s(&file, "CONOUT$", "w", stderr);
    }

    gBrowser = std::make_unique<Browser>(frame);
    gBrowser->LoadSettings();

    frame->SetCurrentTab(ServerListType::FAVORITES);

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
        return false;

    // gBrowser->SaveSettings();

    return true;
}
