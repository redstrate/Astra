include(FetchContent)

FetchContent_Declare(
        qtkeychain
        GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
        GIT_TAG        v0.12.0
)

set(BUILD_WITH_QT6 ON CACHE BOOL "" FORCE)
set(QTKEYCHAIN_STATIC ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(qtkeychain)

FetchContent_Declare(
        mbedtls
        GIT_REPOSITORY https://github.com/ARMmbed/mbedtls.git
        GIT_TAG        v2.27.0 # last version with blowfish support
)

FetchContent_MakeAvailable(mbedtls)