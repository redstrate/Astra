set(HEADERS
        include/tabletinterface.h)

set(SRC
        src/tabletinterface.cpp)

add_library(astra_tablet STATIC ${HEADERS} ${SRC} qml/qml.qrc)
target_include_directories(astra_tablet PUBLIC include)
target_link_libraries(astra_tablet PUBLIC
        astra_core
        Qt5::Core
        Qt5::Network
        Qt5::Quick)