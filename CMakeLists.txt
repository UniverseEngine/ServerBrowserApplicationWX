set(SOURCES
    "src/dialogs/AboutDialog.cpp"
    "src/dialogs/AddServerDialog.cpp"
    "src/dialogs/SettingsDialog.cpp"
    "src/Browser.cpp"
    "src/main.cpp"
    "src/ServerBrowserApplication.cpp"
    "src/MyFrame.cpp"
    "ServerBrowserApplicationWX.rc"
    "ServerBrowserApplicationWX.manifest"
)

add_executable(ServerBrowserApplicationWX WIN32 ${SOURCES})
target_include_directories(ServerBrowserApplicationWX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(ServerBrowserApplicationWX PUBLIC _UNICODE)
target_link_libraries(ServerBrowserApplicationWX CoreLibrary LauncherLibrary cpr::cpr nlohmann_json::nlohmann_json wxmono)

install(TARGETS ServerBrowserApplicationWX RUNTIME DESTINATION "Client" COMPONENT LCMPClient)

install(FILES "${CMAKE_SOURCE_DIR}/dependencies/curl/cacert.pem" DESTINATION "Client" COMPONENT LCMPClient)
