cmake_minimum_required(VERSION 3.0)
project(Astra)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Widgets Network Quick CONFIG REQUIRED)

option(ENABLE_WATCHDOG "Build with Tesseract support (needed for Watchdog)" OFF)
option(USE_OWN_LIBRARIES "Build with own libraries" OFF)
option(BUILD_FLATPAK "Build with Flatpak support in mind" OFF)
option(USE_STEAM "Build with Steam support" OFF)
option(USE_GAMEMODE "Build with GameMode support" ON)

if (ENABLE_WATCHDOG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(TESSERACT REQUIRED tesseract)
    pkg_search_module(LEPTONICA REQUIRED lept)
endif ()

if (USE_GAMEMODE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GAMEMODE REQUIRED gamemode)
endif()

include(FetchContent)

if(USE_STEAM)
    add_library(Steamworks IMPORTED STATIC)
    set_target_properties(Steamworks PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${STEAMWORKS_INCLUDE_DIR}
            IMPORTED_LOCATION ${STEAMWORKS_LIBRARIES})
endif()

find_package(Qt5Keychain)
set(LIBRARIES Qt5Keychain::Qt5Keychain ${LIBRARIES})

find_package(QuaZip-Qt5)
set(LIBRARIES QuaZip::QuaZip ${LIBRARIES})

find_package(fmt)
set(LIBRARIES fmt::fmt ${LIBRARIES})

add_subdirectory(external)

add_subdirectory(launcher)