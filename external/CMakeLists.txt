include(FetchContent)

if(NOT TARGET Qt5Keychain::Qt5Keychain)
    message("Using built-in qt keychain")

    FetchContent_Declare(
            qtkeychain
            GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
            GIT_TAG        v0.12.0
    )

    set(BUILD_WITH_QT6 OFF CACHE BOOL "" FORCE)
    set(QTKEYCHAIN_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(qtkeychain)

    set(LIBRARIES qt5keychain ${LIBRARIES} PARENT_SCOPE)
    set(KEYCHAIN_INCLUDE_DIRS
            ${CMAKE_BINARY_DIR}/_deps/qtkeychain-src
            ${CMAKE_BINARY_DIR}/_deps/qtkeychain-build PARENT_SCOPE)
endif()


if(NOT TARGET QuaZip::QuaZip)
    message("Using built-in quazip")

    FetchContent_Declare(
            quazip
            GIT_REPOSITORY https://github.com/stachenov/quazip.git
            GIT_TAG        v1.2
    )
    FetchContent_MakeAvailable(quazip)

    set(LIBRARIES QuaZip ${LIBRARIES} PARENT_SCOPE)
    set(QUAZIP_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/_deps/quazip-src/quazip PARENT_SCOPE)
endif()
