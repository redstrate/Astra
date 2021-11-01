cmake_minimum_required(VERSION 3.0)
project(xivlauncher)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Widgets Network CONFIG REQUIRED)

add_subdirectory(external)

add_executable(xivlauncher
        src/main.cpp
        src/xivlauncher.cpp
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/settingswindow.cpp)

target_link_libraries(xivlauncher Qt6::Core Qt6::Widgets Qt6::Network qt6keychain)

# disgusting, thanks qtkeychain
target_include_directories(xivlauncher PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/qtkeychain-src
        ${CMAKE_BINARY_DIR}/_deps/qtkeychain-build)