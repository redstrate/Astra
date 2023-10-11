# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(astra_static STATIC)

ecm_qt_declare_logging_category(astra_static
        HEADER astra_log.h
        IDENTIFIER ASTRA_LOG
        CATEGORY_NAME zone.xiv.astra
        DESCRIPTION "Astra logs"
        EXPORT ASTRA
)

ecm_qt_declare_logging_category(astra_static
        HEADER astra_http_log.h
        IDENTIFIER ASTRA_HTTP
        CATEGORY_NAME zone.xiv.astra.http
        DESCRIPTION "Astra HTTP requests"
        EXPORT ASTRA
)

ecm_qt_declare_logging_category(astra_static
        HEADER astra_patcher_log.h
        IDENTIFIER ASTRA_PATCHER
        CATEGORY_NAME zone.xiv.astra.patcher
        DESCRIPTION "Astra patcher"
        EXPORT ASTRA
)

target_sources(astra_static PRIVATE
        include/patchlist.h
        src/patchlist.cpp)
target_include_directories(astra_static PUBLIC include)
target_link_libraries(astra_static PUBLIC
        Qt6::Core)

add_executable(astra)

qt_add_qml_module(astra
        URI zone.xiv.astra
        VERSION 1.0)

target_sources(astra PRIVATE
        include/account.h
        include/accountmanager.h
        include/assetupdater.h
        include/compatibilitytoolinstaller.h
        include/encryptedarg.h
        include/gameinstaller.h
        include/headline.h
        include/launchercore.h
        include/launchersettings.h
        include/logger.h
        include/patcher.h
        include/processlogger.h
        include/profile.h
        include/profilemanager.h
        include/sapphirelauncher.h
        include/squareboot.h
        include/squarelauncher.h
        include/steamapi.h
        include/utility.h

        src/account.cpp
        src/accountmanager.cpp
        src/assetupdater.cpp
        src/compatibilitytoolinstaller.cpp
        src/encryptedarg.cpp
        src/gameinstaller.cpp
        src/launchercore.cpp
        src/launchersettings.cpp
        src/logger.cpp
        src/main.cpp
        src/patcher.cpp
        src/processlogger.cpp
        src/profile.cpp
        src/profilemanager.cpp
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/steamapi.cpp
        src/utility.cpp
)

qt_target_qml_sources(astra
    QML_FILES
        ui/Components/FormFileDelegate.qml
        ui/Components/FormFolderDelegate.qml
        ui/Pages/AutoLoginPage.qml
        ui/Pages/BrowserPage.qml
        ui/Pages/LoginPage.qml
        ui/Pages/MainPage.qml
        ui/Pages/NewsPage.qml
        ui/Pages/StatusPage.qml
        ui/Settings/AboutPage.qml
        ui/Settings/AccountSettings.qml
        ui/Settings/AccountsPage.qml
        ui/Settings/CompatibilityToolSetup.qml
        ui/Settings/DeveloperSettings.qml
        ui/Settings/GeneralSettings.qml
        ui/Settings/ProfileSettings.qml
        ui/Settings/ProfilesPage.qml
        ui/Settings/SettingsPage.qml
        ui/Setup/AccountSetup.qml
        ui/Setup/AddSapphire.qml
        ui/Setup/AddSquareEnix.qml
        ui/Setup/DownloadSetup.qml
        ui/Setup/ExistingSetup.qml
        ui/Setup/InstallProgress.qml
        ui/Setup/SetupPage.qml
        ui/Main.qml
)

set_source_files_properties(../zone.xiv.astra.svg PROPERTIES
        QT_RESOURCE_ALIAS /zone.xiv.astra.svg
)

qt_target_qml_sources(astra
        PREFIX /
        RESOURCES
        ../zone.xiv.astra.svg
)

kconfig_add_kcfg_files(astra GENERATE_MOC config.kcfgc accountconfig.kcfgc profileconfig.kcfgc)
target_include_directories(astra PRIVATE include ${CMAKE_BINARY_DIR})
target_link_libraries(astra PRIVATE
        astra_static
        physis
        physis-logger
        cotp
        QuaZip::QuaZip
        Qt6Keychain::Qt6Keychain
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::WebView
        Qt6::Concurrent
        KF6::Kirigami2
        KF6::I18n
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::CoreAddons
        QCoro::Core
        QCoro::Network
        QCoro::Qml)

if (BUILD_FLATPAK)
    target_compile_definitions(astra PRIVATE FLATPAK)
endif ()

if (ENABLE_STEAM)
    target_link_libraries(astra PRIVATE Steamworks)
    target_compile_definitions(astra PRIVATE ENABLE_STEAM)
endif ()

if (ENABLE_GAMEMODE)
    target_link_libraries(astra PRIVATE ${GAMEMODE_LIBRARIES})
    target_compile_definitions(astra PRIVATE ENABLE_GAMEMODE)
endif ()

install(TARGETS astra ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if (WIN32)
    get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET astra
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:astra>\"
            )
endif ()

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
