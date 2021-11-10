cmake_minimum_required(VERSION 3.0)
project(xivlauncher)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Widgets Network CONFIG REQUIRED)

add_subdirectory(external)

add_executable(xivlauncher
        src/main.cpp
        src/xivlauncher.cpp
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/settingswindow.cpp
        src/blowfish.cpp)

target_link_libraries(xivlauncher Qt6::Core Qt6::Widgets Qt6::Network qt6keychain)

# disgusting, thanks qtkeychain
target_include_directories(xivlauncher PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/qtkeychain-src
        ${CMAKE_BINARY_DIR}/_deps/qtkeychain-build)

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