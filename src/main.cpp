#include "main.hpp"

#include "MyApp.hpp"

using namespace Universe;

extern "C" int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wxCmdLineArgType lpCmdLine, int nCmdShow)
{
    wxDisableAsserts();
    wxLog::SetLogLevel(wxLOG_Info);
    return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
MyApp& wxGetApp()
{
    return *static_cast<MyApp*>(wxApp::GetInstance());
}
wxAppConsole* wxCreateApp()
{
    wxAppConsole::CheckBuildOptions("3"
                                    "."
                                    "3"
                                    "."
                                    "0"
                                    " ("
                                    "wchar_t"
                                    ",Visual C++ "
                                    "1900"
                                    ",STL containers"
                                    ",compatible with 3.2"
                                    ")",
        "your program");
    return new MyApp;
}
wxAppInitializer wxTheAppInitializer((wxAppInitializerFunction)wxCreateApp);
