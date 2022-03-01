include(FetchContent)

find_package(QtKeychain-Qt5)

if(NOT TARGET Qt5Keychain::Qt5Keychain)
    FetchContent_Declare(
            qtkeychain
            GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
            GIT_TAG        v0.12.0
    )

    set(BUILD_WITH_QT6 OFF CACHE BOOL "" FORCE)
    set(QTKEYCHAIN_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(qtkeychain)

    set(KEYCHAIN_LIBRARIES qt5keychain PARENT_SCOPE)
    set(KEYCHAIN_INCLUDE_DIRS
            ${CMAKE_BINARY_DIR}/_deps/qtkeychain-src
            ${CMAKE_BINARY_DIR}/_deps/qtkeychain-build PARENT_SCOPE)
else()
    set(KEYCHAIN_LIBRARIES Qt5Keychain::Qt5Keychain PARENT_SCOPE)
endif()


#quazip
find_package(QuaZip-Qt5)
if(NOT TARGET QuaZip::QuaZip)
    FetchContent_Declare(
            quazip
            GIT_REPOSITORY https://github.com/stachenov/quazip.git
            GIT_TAG        v1.2
    )
    FetchContent_MakeAvailable(quazip)

    set(QUAZIP_LIBRARIES QuaZip PARENT_SCOPE)
    set(QUAZIP_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/_deps/quazip-src/quazip PARENT_SCOPE)
else()
    set(QUAZIP_LIBRARIES QuaZip::QuaZip PARENT_SCOPE)
endif()
