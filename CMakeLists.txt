# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.25)
project(Astra VERSION 0.5.0 LANGUAGES CXX)

# build options used for distributors
option(BUILD_FLATPAK "Build for Flatpak." OFF)

# options for features you may want or need
option(ENABLE_STEAM "Build with Steam support, requires supplying the Steam SDK yourself." OFF)
option(ENABLE_GAMEMODE "Build with Feral GameMode support, requires the daemon to be installed." ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION 6.5)
set(KF_MIN_VERSION 5.240)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(ECMFindQmlModule)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(KDEGitCommitHooks)
include(KDEClangFormat)
include(ECMAddTests)

ecm_setup_version(${PROJECT_VERSION}
        VARIABLE_PREFIX ASTRA
        VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/astra-version.h
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
        Core
        Widgets
        Network
        QuickControls2
        WebView
        Concurrent
        Test)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n Config CoreAddons)
find_package(PkgConfig REQUIRED)
find_package(QCoro6 REQUIRED COMPONENTS Core Network Qml)
qcoro_enable_coroutines()

qt_policy(SET QTP0001 NEW)

if (ENABLE_GAMEMODE)
    pkg_search_module(GAMEMODE REQUIRED gamemode)
endif ()

if (ENABLE_STEAM)
    add_library(Steamworks IMPORTED SHARED)
    set_target_properties(Steamworks PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${STEAMWORKS_INCLUDE_DIR}
            IMPORTED_LOCATION ${STEAMWORKS_LIBRARIES})

    if (BUILD_FLATPAK)
        install(IMPORTED_RUNTIME_ARTIFACTS Steamworks)
    endif ()
endif ()

find_package(Qt6Keychain REQUIRED)
find_package(QuaZip-Qt6 REQUIRED)

add_subdirectory(external)
add_subdirectory(launcher)

install(FILES zone.xiv.astra.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zone.xiv.astra.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES zone.xiv.astra.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)