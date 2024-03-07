sources = {
    "src/dialogs/AboutDialog.cpp",
    "src/dialogs/AddServerDialog.cpp",
    "src/dialogs/SettingsDialog.cpp",
    "src/Browser.cpp",
    "src/main.cpp",
    "src/MyApp.cpp",
    "src/MyFrame.cpp",
    "ServerBrowserApplicationWX.rc",
    "ServerBrowserApplicationWX.manifest"
}

target("ServerBrowserApplicationWX")
    set_kind("binary")
    add_rules("win.sdk.application")
    add_files(sources)
    add_deps("LauncherLibrary", "CoreLibrary")
    add_packages("tinyxml2", "fmt", "eventpp", "libcurl", "wxwidgets", "nlohmann_json")
    add_includedirs("include")
    set_pcxxheader("src/pch.hpp")
    add_defines("_UNICODE")
