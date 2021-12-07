cmake_minimum_required(VERSION 3.0)
project(xivlauncher)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Widgets Network CONFIG REQUIRED)

add_subdirectory(external)

set(SRC
        src/main.cpp
        src/launchercore.cpp
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/settingswindow.cpp
        src/blowfish.cpp
        src/assetupdater.cpp
        src/assetupdater.h
        src/launcherwindow.cpp
        src/launcherwindow.h
        src/watchdog.h
        src/watchdog.cpp)

set(LIBRARIES
        Qt5::Core Qt5::Widgets Qt5::Network qt5keychain QuaZip)

if(UNIX)
    set(SRC ${SRC}
            src/gameparser.h
            src/gameparser.cpp)

    set(LIBRARIES ${LIBRARIES}
            tesseract
            lept
            X11
            Xcomposite
            Xrender)
endif()

add_executable(xivlauncher ${SRC})

target_link_libraries(xivlauncher PUBLIC ${LIBRARIES})

# disgusting, thanks qtkeychain and quazip
target_include_directories(xivlauncher PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/qtkeychain-src
        ${CMAKE_BINARY_DIR}/_deps/qtkeychain-build
        ${CMAKE_BINARY_DIR}/_deps/quazip-src)

install(TARGETS xivlauncher
        DESTINATION "${INSTALL_BIN_PATH}"
        )

if(WIN32)
    get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET xivlauncher
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:xivlauncher>\"
            )
endif()