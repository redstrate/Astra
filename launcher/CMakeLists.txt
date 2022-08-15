add_subdirectory(core)
add_subdirectory(cli)
add_subdirectory(desktop)
add_subdirectory(tablet)

add_executable(astra
        main.cpp
        tablet/qml/qml.qrc)

target_link_libraries(astra PUBLIC
        ${LIBRARIES}
        astra_core
        astra_desktop
        astra_cli
        astra_tablet)
target_compile_features(astra PUBLIC cxx_std_17)
set_target_properties(astra PROPERTIES CXX_EXTENSIONS OFF)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE LICENSE_TXT)
STRING(REPLACE "\n" " \\n" LICENSE_TXT ${LICENSE_TXT})
STRING(REPLACE "\"" "\"\"" LICENSE_TXT ${LICENSE_TXT})

configure_file(${CMAKE_CURRENT_LIST_DIR}/../cmake/license.h.in
        ${CMAKE_BINARY_DIR}/license.h)

if (BUILD_FLATPAK)
    target_compile_definitions(astra PRIVATE FLATPAK)
endif ()

install(TARGETS astra
        DESTINATION "${INSTALL_BIN_PATH}")

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
