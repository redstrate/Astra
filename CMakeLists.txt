cmake_minimum_required(VERSION 3.0)
project(Astra)

# build options used for distributors
option(BUILD_FLATPAK "Build for Flatpak." OFF)

# options for features you may want or need
option(ENABLE_WATCHDOG "Build support for Watchdog, requires an X11 system." OFF)
option(ENABLE_STEAM "Build with Steam support, requires supplying the Steam SDK." OFF)
option(ENABLE_GAMEMODE "Build with Feral GameMode support, requires the daemon to be installed." ON)
option(ENABLE_TABLET "Build support for the tablet interface, meant for devices like the Steam Deck." ON)
option(ENABLE_DESKTOP "Build support for the desktop interface, meant to be used on desktops and laptops." ON)
option(ENABLE_CLI "Build support for the command-line interface, meant for scripting and automation." ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Widgets Network Quick CONFIG REQUIRED)

if (ENABLE_WATCHDOG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(TESSERACT REQUIRED tesseract)
    pkg_search_module(LEPTONICA REQUIRED lept)
endif ()

if (ENABLE_GAMEMODE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GAMEMODE REQUIRED gamemode)
endif ()

if (ENABLE_STEAM)
    add_library(Steamworks IMPORTED SHARED)
    set_target_properties(Steamworks PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${STEAMWORKS_INCLUDE_DIR}
            IMPORTED_LOCATION ${STEAMWORKS_LIBRARIES})
endif ()

find_package(Qt5Keychain REQUIRED)
find_package(QuaZip-Qt5 REQUIRED)

add_subdirectory(external)
add_subdirectory(launcher)