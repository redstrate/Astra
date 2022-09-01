cmake_minimum_required(VERSION 3.5)
project(cotp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
find_package(Gcrypt 1.6.0 REQUIRED)

include_directories(${GCRYPT_INCLUDE_DIR})

link_directories(${GCRYPT_LIBRARY_DIRS})

# set up versioning.
set(BUILD_MAJOR "1")
set(BUILD_MINOR "2")
set(BUILD_VERSION "6")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_VERSION})

set(CMAKE_C_STANDARD 11)

set(COTP_HEADERS src/cotp.h)
set(SOURCE_FILES src/otp.c)

set(CMAKE_C_FLAGS "-Wall -Wextra -O3 -Wno-format-truncation -fstack-protector-strong -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3")

add_library(cotp SHARED ${SOURCE_FILES})

target_link_libraries(cotp ${GCRYPT_LIBRARIES} baseencode)
target_include_directories(cotp PUBLIC src)

set_target_properties(cotp PROPERTIES VERSION ${BUILD_VERSION} SOVERSION ${BUILD_MAJOR}${BUILD_MINOR})