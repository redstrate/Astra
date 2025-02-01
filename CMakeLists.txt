# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.25)
project(Astra VERSION 0.6.0 LANGUAGES CXX)

# build options used for distributors
option(BUILD_FLATPAK "Build for Flatpak." OFF)
option(BUILD_WEBVIEW "Build support for the integrated web browser. Only used on the Steam Deck." OFF)

# TODO: phase out once all supported builds can turn on this option
option(BUILD_SYNC "Build Matrix sync support." OFF)

# options for features you may want or need
if (NOT WIN32 AND NOT APPLE)
    option(ENABLE_GAMEMODE "Build with Feral GameMode support, requires the daemon to be installed." ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION 6.8)
set(KF_MIN_VERSION 6.10)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(KDEGitCommitHooks)
include(KDEClangFormat)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(ECMAddAppIcon)
include(ECMQmlModule)

ecm_setup_version(${PROJECT_VERSION}
        VARIABLE_PREFIX ASTRA
        VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/astra-version.h
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
        Core
        Widgets
        Network
        QuickControls2
        Concurrent
        Test)
if (BUILD_WEBVIEW)
    find_package(Qt6WebView ${QT_MIN_VERSION} REQUIRED)
endif ()
if (LINUX)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS DBus)
endif ()

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami I18n Config CoreAddons Archive IconThemes)
find_package(KF6KirigamiAddons 1.7.0 REQUIRED)
find_package(QCoro6 REQUIRED COMPONENTS Core Network Qml)
qcoro_enable_coroutines()
if (BUILD_SYNC)
    find_package(QuotientQt6 REQUIRED)
endif()

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

if (ENABLE_GAMEMODE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GAMEMODE REQUIRED gamemode)
endif ()

find_package(Qt6Keychain REQUIRED)

# TODO: we should really do this on all platforms anyway
if (WIN32)
    find_package(KF6BreezeIcons REQUIRED)
endif ()

add_subdirectory(external)
add_subdirectory(launcher)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

if (NOT WIN32)
    install(FILES zone.xiv.astra.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.astra.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
    install(FILES zone.xiv.astra.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

    ecm_qt_install_logging_categories(
            EXPORT ASTRA
            FILE astra.categories
            SORT DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
    )
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
