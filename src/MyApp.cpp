#include "pch.hpp"

#include "MyApp.hpp"
#include "MyFrame.hpp"

#include "Browser.hpp"

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);

    wchar_t _path[MAX_PATH];
    GetModuleFileName(NULL, _path, MAX_PATH);
    std::filesystem::current_path(Path(_path).remove_filename());
    std::filesystem::create_directories(std::filesystem::current_path() / "logs");

    Logger::Initialise("logs/browser.log");

    gBrowser = std::make_unique<Browser>(frame);
    gBrowser->LoadSettings();

    frame->SetCurrentTab(ListViewTab::FAVORITES);

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
        return false;

    // gBrowser->SaveSettings();

    return true;
}