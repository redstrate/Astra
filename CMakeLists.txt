cmake_minimum_required(VERSION 3.0)
project(Astra)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Widgets Network Quick CONFIG REQUIRED)

option(ENABLE_WATCHDOG "Build with Tesseract support (needed for Watchdog)" OFF)
option(USE_OWN_LIBRARIES "Build with own libraries" OFF)
option(BUILD_FLATPAK "Build with Flatpak support in mind" OFF)

if(ENABLE_WATCHDOG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(TESSERACT REQUIRED tesseract)
    pkg_search_module(LEPTONICA REQUIRED lept)
endif()

include(FetchContent)

if(NOT USE_OWN_LIBRARIES)
    find_package(Qt5Keychain QUIET)
endif()

if(TARGET Qt5Keychain::Qt5Keychain)
    message("Using system library for Qt5 Keychain")

    set(LIBRARIES Qt5Keychain::Qt5Keychain ${LIBRARIES})
    set(KEYCHAIN_INCLUDE_DIRS ${QTKEYCHAIN_INCLUDE_DIRS}/qt5keychain) # this is to be consistent with the built-in lib?
else()
    message("Using downloaded qtkeychain")

    FetchContent_Declare(
            qtkeychain
            GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
            GIT_TAG        v0.12.0
    )

    set(BUILD_WITH_QT6 OFF CACHE BOOL "" FORCE)
    set(QTKEYCHAIN_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(qtkeychain)

    set(LIBRARIES qt5keychain ${LIBRARIES})
    set(KEYCHAIN_INCLUDE_DIRS
            ${CMAKE_BINARY_DIR}/_deps/qtkeychain-src
            ${CMAKE_BINARY_DIR}/_deps/qtkeychain-build)
endif()

if(NOT USE_OWN_LIBRARIES)
    find_package(QuaZip-Qt5 QUIET)
endif()

if(TARGET QuaZip::QuaZip)
    message("Using system library for Quazip")

    set(LIBRARIES QuaZip::QuaZip ${LIBRARIES})
else()
    message("Using downloaded quazip")

    FetchContent_Declare(
            quazip
            GIT_REPOSITORY https://github.com/stachenov/quazip.git
            GIT_TAG        v1.2
    )

    set(QUAZIP_USE_QT_ZLIB ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(quazip)

    set(LIBRARIES QuaZip ${LIBRARIES})
    set(QUAZIP_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/_deps/quazip-src/quazip)
endif()

if(NOT USE_OWN_LIBRARIES)
    find_package(fmt QUIET)
endif()

if(TARGET fmt::fmt)
    message("Using system library for fmt")

    set(LIBRARIES fmt::fmt ${LIBRARIES})
else()
    message("Using downloaded fmt")

    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt
            GIT_TAG        master
    )

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(fmt)

    set(LIBRARIES fmt::fmt ${LIBRARIES})
endif()

add_subdirectory(external)

if(ENABLE_WATCHDOG)
    set(LIBRARIES ${LIBRARIES} ${TESSERACT_LIBRARIES} ${LEPTONICA_LIBRARIES})

    set(SRC ${SRC}
        include/watchdog.h
        src/watchdog.cpp
        include/gameparser.h
        src/gameparser.cpp)
endif()

if(ENABLE_WATCHDOG)
    set(LIBRARIES ${LIBRARIES}
            X11
            Xcomposite
            Xrender)
endif()

add_subdirectory(launcher)