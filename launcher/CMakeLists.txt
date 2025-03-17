# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(astra_static STATIC)

ecm_add_qml_module(astra_static GENERATE_PLUGIN_SOURCE
        URI zone.xiv.astra
        VERSION 1.0)

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
        include/account.h
        include/processwatcher.h
        include/profilemanager.h
        include/profile.h
        include/utility.h
        include/accountmanager.h
        include/assetupdater.h
        include/benchmarkinstaller.h
        include/compatibilitytoolinstaller.h
        include/encryptedarg.h
        include/existinginstallmodel.h
        include/gamerunner.h
        include/gameinstaller.h
        include/headline.h
        include/launchercore.h
        include/patcher.h
        include/processlogger.h
        include/sapphirelogin.h
        include/squareenixlogin.h
        include/steamapi.h

        src/accountmanager.cpp
        src/assetupdater.cpp
        src/benchmarkinstaller.cpp
        src/compatibilitytoolinstaller.cpp
        src/encryptedarg.cpp
        src/existinginstallmodel.cpp
        src/gamerunner.cpp
        src/headline.cpp
        src/gameinstaller.cpp
        src/launchercore.cpp
        src/account.cpp
        src/processwatcher.cpp
        src/profilemanager.cpp
        src/profile.cpp
        src/utility.cpp
        src/patcher.cpp
        src/processlogger.cpp
        src/sapphirelogin.cpp
        src/squareenixlogin.cpp
        src/steamapi.cpp)
target_include_directories(astra_static PUBLIC include)
target_link_libraries(astra_static PUBLIC
        physis
        cotp
        Qt6::Core
        Qt6::Quick
        Qt6::Network
        Qt6::Concurrent
        Qt6Keychain::Qt6Keychain
        KF6::I18n
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::Archive
        QCoro::Core
        QCoro::Network
        QCoro::Qml)
kconfig_target_kcfg_file(astra_static FILE config.kcfg CLASS_NAME Config MUTATORS GENERATE_PROPERTIES GENERATE_MOC DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR QML_REGISTRATION QML_UNCREATABLE USE_ENUM_TYPES)
kconfig_target_kcfg_file(astra_static FILE accountconfig.kcfg CLASS_NAME AccountConfig MUTATORS GENERATE_PROPERTIES GENERATE_MOC DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR QML_REGISTRATION QML_UNCREATABLE USE_ENUM_TYPES)
kconfig_target_kcfg_file(astra_static FILE profileconfig.kcfg CLASS_NAME ProfileConfig MUTATORS GENERATE_PROPERTIES GENERATE_MOC DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR QML_REGISTRATION QML_UNCREATABLE)
target_include_directories(astra_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

if (NOT MSVC)
    target_compile_options(astra_static PUBLIC -fexceptions)
endif ()

if (BUILD_FLATPAK)
    target_compile_definitions(astra_static PUBLIC FLATPAK)
endif ()

if (ENABLE_GAMEMODE)
    target_link_libraries(astra_static PUBLIC ${GAMEMODE_LIBRARIES})
    target_compile_definitions(astra_static PUBLIC ENABLE_GAMEMODE)
endif ()

if (BUILD_SYNC)
    target_sources(astra_static PRIVATE
            include/charactersync.h
            include/syncmanager.h

            src/charactersync.cpp
            src/syncmanager.cpp
    )
    target_compile_definitions(astra_static PUBLIC BUILD_SYNC)
    target_link_libraries(astra_static PRIVATE QuotientQt6)
endif ()

# For screensaver inhibition on Linux
if (TARGET Qt6::DBus)
    target_link_libraries(astra_static PRIVATE Qt6::DBus)
    target_compile_definitions(astra_static PRIVATE -DHAS_DBUS)
endif ()

add_executable(astra)

target_sources(astra PRIVATE
        include/logger.h

        src/logger.cpp
        src/main.cpp
)

qt_target_qml_sources(astra_static
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
        ui/Settings/SyncSettings.qml
        ui/Setup/AccountSetup.qml
        ui/Setup/AddSapphire.qml
        ui/Setup/AddSquareEnix.qml
        ui/Setup/BenchmarkInstallProgress.qml
        ui/Setup/ExistingSetup.qml
        ui/Setup/InstallProgress.qml
        ui/Setup/SetupPage.qml
        ui/Main.qml
)

set_source_files_properties(../zone.xiv.astra.svg PROPERTIES
        QT_RESOURCE_ALIAS /zone.xiv.astra.svg
)

qt_target_qml_sources(astra_static
        PREFIX /
        RESOURCES
        ../zone.xiv.astra.svg
)

target_link_libraries(astra PRIVATE
        astra_static
        astra_staticplugin
        physis-logger
        KDAB::kdsingleapplication
        Qt6::Widgets
        Qt6::QuickControls2
        KF6::Kirigami
        KF6::IconThemes)

if (BUILD_WEBVIEW)
    target_link_libraries(astra PRIVATE
            Qt6::WebView
    )
    target_compile_definitions(astra PRIVATE HAVE_WEBVIEW)
endif ()

if (WIN32)
    set_target_properties(astra PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "Astra")
endif ()

install(TARGETS astra ${KF6_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_add_app_icon(astra
        ICONS
        ${CMAKE_SOURCE_DIR}/resources/16-astra.png
        ${CMAKE_SOURCE_DIR}/resources/32-astra.png
        ${CMAKE_SOURCE_DIR}/resources/48-astra.png
        ${CMAKE_SOURCE_DIR}/resources/256-astra.png
)

qt_finalize_target(astra)

# the current tooling completely breaks in Flatpak, and is unnecessary there anyway
if (WIN32)
    qt_generate_deploy_qml_app_script(
            TARGET astra
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif ()
