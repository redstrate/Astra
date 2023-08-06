# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(libbaseencode)
add_subdirectory(libcotp)

include(FetchContent)

FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.4.2
)
FetchContent_MakeAvailable(Corrosion)

FetchContent_Declare(
        libphysis
        GIT_REPOSITORY https://git.sr.ht/~redstrate/libphysis
        GIT_TAG        main
)
FetchContent_MakeAvailable(libphysis)

corrosion_import_crate(MANIFEST_PATH ${libphysis_SOURCE_DIR}/Cargo.toml
        FEATURES game_install)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)

target_include_directories(physis INTERFACE ${libphysis_SOURCE_DIR}/target/public)
target_link_libraries(physis INTERFACE ${UNSHIELD_LIBRARIES})
target_link_directories(physis INTERFACE ${UNSHIELD_LIBRARY_DIRS})
