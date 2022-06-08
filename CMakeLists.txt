cmake_minimum_required(VERSION 3.0)
project(Astra)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets Network CONFIG REQUIRED)

option(ENABLE_WATCHDOG "Build with Tesseract support (needed for Watchdog)" OFF)
option(USE_OWN_LIBRARIES "Build with own libraries" OFF)
option(BUILD_FLATPAK "Build with Flatpak support in mind" OFF)

if(ENABLE_WATCHDOG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(TESSERACT REQUIRED tesseract)
    pkg_search_module(LEPTONICA REQUIRED lept)
endif()

set(SRC
        src/main.cpp
        src/launchercore.cpp
        include/launchercore.h
        include/squarelauncher.h
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/settingswindow.cpp
        src/blowfish.cpp
        src/assetupdater.cpp
        include/assetupdater.h
        src/launcherwindow.cpp
        include/launcherwindow.h
        src/gamescopesettingswindow.cpp
        include/gamescopesettingswindow.h
        include/headline.h
        src/headline.cpp
        include/config.h
        include/gameinstaller.h
        src/gameinstaller.cpp
        src/encryptedarg.cpp
        src/aboutwindow.cpp
        include/aboutwindow.h
        src/bannerwidget.cpp
        include/bannerwidget.h
        include/desktopinterface.h
        include/cmdinterface.h
        src/cmdinterface.cpp
        src/desktopinterface.cpp)

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

set(LIBRARIES
        Qt5::Core Qt5::Widgets Qt5::Network ${LIBRARIES})

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

add_executable(astra ${SRC})

target_link_libraries(astra PUBLIC ${LIBRARIES} libxiv)
target_compile_features(astra PUBLIC cxx_std_17)
set_target_properties(astra PROPERTIES CXX_EXTENSIONS OFF)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE LICENSE_TXT)
STRING(REPLACE "\n" " \\n" LICENSE_TXT ${LICENSE_TXT})
STRING(REPLACE "\"" "\"\"" LICENSE_TXT ${LICENSE_TXT})

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/license.h.in
        ${CMAKE_BINARY_DIR}/license.h)

target_include_directories(astra
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${KEYCHAIN_INCLUDE_DIRS}
        ${QUAZIP_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR})

if(ENABLE_WATCHDOG)
    target_include_directories(astra PRIVATE ${TESSERACT_INCLUDE_DIRS} ${LEPTONICA_INCLUDE_DIRS})

    target_compile_definitions(astra PRIVATE ENABLE_WATCHDOG)
endif()

if(BUILD_FLATPAK)
    target_compile_definitions(astra PRIVATE FLATPAK)
endif()

install(TARGETS astra
        DESTINATION "${INSTALL_BIN_PATH}")

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET astra
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:astra>\"
            )
endif()
