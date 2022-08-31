set(HEADERS
        include/aboutwindow.h
        include/assetupdater.h
        include/bannerwidget.h
        include/desktopinterface.h
        include/gamescopesettingswindow.h
        include/launcherwindow.h
        include/settingswindow.h
        include/autologinwindow.h)

set(SRC
        src/aboutwindow.cpp
        src/assetupdater.cpp
        src/bannerwidget.cpp
        src/desktopinterface.cpp
        src/gamescopesettingswindow.cpp
        src/launcherwindow.cpp
        src/settingswindow.cpp
        src/autologinwindow.cpp)

add_library(astra_desktop STATIC ${HEADERS} ${SRC})
target_include_directories(astra_desktop PUBLIC include)
target_link_libraries(astra_desktop PUBLIC
        astra_core
        Qt5::Core
        Qt5::Widgets
        Qt5::Network)