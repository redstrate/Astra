set(HEADERS
        include/cmdinterface.h)

set(SRC
        src/cmdinterface.cpp)

add_library(astra_cli STATIC ${HEADERS} ${SRC})
target_include_directories(astra_cli PUBLIC include)
target_link_libraries(astra_cli PUBLIC
        astra_core
        Qt5::Core
        Qt5::Network)