cmake_minimum_required(VERSION 3.5)
project(baseencode)

include(GNUInstallDirs)

# set up versioning.
set(BUILD_MAJOR "1")
set(BUILD_MINOR "0")
set(BUILD_VERSION "14")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_VERSION})

set(CMAKE_C_STANDARD 11)

set(BASEENCODE_HEADERS src/baseencode.h)
set(SOURCE_FILES src/base32.c src/base64.c)

set(CMAKE_C_FLAGS "-Wall -Werror -fPIC")

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${BUILD_VERSION} SOVERSION ${BUILD_MAJOR})