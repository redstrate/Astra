include(FetchContent)

FetchContent_Declare(
        qtkeychain
        GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
        GIT_TAG        v0.12.0
)

set(BUILD_WITH_QT6 OFF CACHE BOOL "" FORCE)
set(QTKEYCHAIN_STATIC ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(qtkeychain)

FetchContent_Declare(
        quazip
        GIT_REPOSITORY https://github.com/stachenov/quazip.git
        GIT_TAG        v1.2
)
FetchContent_MakeAvailable(quazip)