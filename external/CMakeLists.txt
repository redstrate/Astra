include(FetchContent)

FetchContent_Declare(
        qtkeychain
        GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
        GIT_TAG        v0.12.0
)

set(BUILD_WITH_QT6 ON)
set(QTKEYCHAIN_STATIC ON)

FetchContent_MakeAvailable(qtkeychain)