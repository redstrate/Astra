# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

# See https://gitlab.kitware.com/cmake/cmake/-/issues/20312
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTS OFF)

add_subdirectory(libcotp EXCLUDE_FROM_ALL)

find_package(Corrosion QUIET)
if (NOT Corrosion_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            Corrosion
            GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
            GIT_TAG v0.5.2
    )

    FetchContent_MakeAvailable(Corrosion)
endif()

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml)

target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)

set(KDSingleApplication_QT6 ON)
set(KDSingleApplication_STATIC ON)
add_subdirectory(kdsingleapplication EXCLUDE_FROM_ALL)
