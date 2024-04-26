# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

# See https://gitlab.kitware.com/cmake/cmake/-/issues/20312
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTS OFF)

add_subdirectory(libcotp EXCLUDE_FROM_ALL)

find_package(Corrosion REQUIRED)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml
        NO_DEFAULT_FEATURES
        FEATURES game_install visual_data) # FIXME: split visual_data? we only need texture decompression
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libphysis/logger EXCLUDE_FROM_ALL)

target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)

# TODO: enable unshield on Windows
if (NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)
    target_link_directories(physis INTERFACE ${UNSHIELD_LIBRARY_DIRS})
endif()

set(KDSingleApplication_QT6 ON)
set(KDSingleApplication_STATIC ON)
add_subdirectory(kdsingleapplication EXCLUDE_FROM_ALL)