# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.25)
project(Astra VERSION 0.5.1 LANGUAGES CXX)

# build options used for distributors
option(BUILD_FLATPAK "Build for Flatpak." OFF)

# options for features you may want or need
option(ENABLE_GAMEMODE "Build with Feral GameMode support, requires the daemon to be installed." ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION 6.6)
set(KF_MIN_VERSION 5.240)

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
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami I18n Config CoreAddons Archive)
find_package(PkgConfig REQUIRED)
find_package(QCoro6 REQUIRED COMPONENTS Core Network Qml)
qcoro_enable_coroutines()

qt_policy(SET QTP0001 NEW)

if (ENABLE_GAMEMODE)
    pkg_search_module(GAMEMODE REQUIRED gamemode)
endif ()

find_package(Qt6Keychain REQUIRED)

add_subdirectory(external)
add_subdirectory(launcher)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(FILES zone.xiv.astra.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zone.xiv.astra.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES zone.xiv.astra.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)