# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(astra)
target_sources(astra PRIVATE
        include/account.h
        include/accountmanager.h
        include/assetupdater.h
        include/compatibilitytoolinstaller.h
        include/encryptedarg.h
        include/gameinstaller.h
        include/headline.h
        include/launchercore.h
        include/patcher.h
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
        src/main.cpp
        src/patcher.cpp
        src/profile.cpp
        src/profilemanager.cpp
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/steamapi.cpp
        src/utility.cpp

        resources.qrc)
kconfig_add_kcfg_files(astra GENERATE_MOC config.kcfgc accountconfig.kcfgc profileconfig.kcfgc)
target_include_directories(astra PRIVATE include ${CMAKE_BINARY_DIR})
target_link_libraries(astra PRIVATE
        physis
        cotp
        crypto
        QuaZip::QuaZip
        Qt6Keychain::Qt6Keychain
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::WebView
        KF6::Kirigami2
        KF6::I18n
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::CoreAddons)

if (ENABLE_WATCHDOG)
    target_sources(astra PRIVATE
            include/gameparser.h
            include/watchdog.h

            src/gameparser.cpp
            src/watchdog.cpp)
    target_link_libraries(astra PRIVATE
            ${TESSERACT_LIBRARIES}
            ${LEPTONICA_LIBRARIES}
            X11
            Xcomposite
            Xrender)

    target_include_directories(astra_core PRIVATE ${TESSERACT_INCLUDE_DIRS} ${LEPTONICA_INCLUDE_DIRS})
    target_compile_definitions(astra_core PRIVATE ENABLE_WATCHDOG)
endif ()

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
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET astra
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:astra>\"
            )
endif ()
