set(HEADERS
        include/config.h
        include/dxvkinstaller.h
        include/encryptedarg.h
        include/gameinstaller.h
        include/headline.h
        include/launchercore.h
        include/sapphirelauncher.h
        include/squareboot.h
        include/squarelauncher.h
        include/patcher.h)

set(SRC
        src/dxvkinstaller.cpp
        src/encryptedarg.cpp
        src/gameinstaller.cpp
        src/headline.cpp
        src/launchercore.cpp
        src/sapphirelauncher.cpp
        src/squareboot.cpp
        src/squarelauncher.cpp
        src/patcher.cpp)

if(ENABLE_WATCHDOG)
    set(HEADERS ${HEADERS}
            include/gameparser.h
            include/watchdog.h)

    set(SRC ${SRC}
            src/gameparser.cpp
            src/watchdog.cpp)
endif()

add_library(astra_core STATIC ${HEADERS} ${SRC})
target_include_directories(astra_core PUBLIC
        ${KEYCHAIN_INCLUDE_DIRS}
        ${QUAZIP_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        include)
target_link_libraries(astra_core PUBLIC
        physis
        z # FIXME: remove!
        ${LIBRARIES}
        Qt5::Core
        Qt5::Network
        Qt5::Widgets  # widgets is required by watchdog, to be fixed/removed later
        Qt5::Quick # required for some type registrations
        PRIVATE
        astra_desktop) # desktop is currently required by the core, to be fixed/removed later

if(ENABLE_WATCHDOG)
    target_include_directories(astra_core PUBLIC ${TESSERACT_INCLUDE_DIRS} ${LEPTONICA_INCLUDE_DIRS})

    target_compile_definitions(astra_core PUBLIC ENABLE_WATCHDOG)
endif()
