# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTS OFF)

add_subdirectory(libcotp)

find_package(Corrosion REQUIRED)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml
        NO_DEFAULT_FEATURES
        FEATURES game_install)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)

target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)
target_link_libraries(physis INTERFACE ${UNSHIELD_LIBRARIES})
target_link_directories(physis INTERFACE ${UNSHIELD_LIBRARY_DIRS})
