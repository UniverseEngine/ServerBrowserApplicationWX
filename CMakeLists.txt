set(SOURCES
    "src/dialogs/AboutDialog.cpp"
    "src/dialogs/AddServerDialog.cpp"
    "src/dialogs/SettingsDialog.cpp"
    "src/Browser.cpp"
    "src/main.cpp"
    "src/MyApp.cpp"
    "src/MyFrame.cpp"
    "src/pch.cpp"
    "ServerBrowserApplicationWX.rc"
    "ServerBrowserApplicationWX.manifest"
)

add_executable(ServerBrowserApplicationWX WIN32 ${SOURCES})
target_precompile_headers(ServerBrowserApplicationWX PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp)
target_include_directories(ServerBrowserApplicationWX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(ServerBrowserApplicationWX PUBLIC _UNICODE)
target_link_libraries(ServerBrowserApplicationWX CoreLibrary LauncherLibrary libcurl nlohmann_json::nlohmann_json wxmono)

install(TARGETS ServerBrowserApplicationWX RUNTIME DESTINATION "Client" COMPONENT LCMPClient)

install(FILES "${CMAKE_SOURCE_DIR}/dependencies/curl/cacert.pem" DESTINATION "Client" COMPONENT LCMPClient)
