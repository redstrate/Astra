add_subdirectory(libbaseencode)
add_subdirectory(libcotp)

include(FetchContent)

FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.2.1
)

FetchContent_MakeAvailable(Corrosion)

FetchContent_Declare(
        libphysis
        GIT_REPOSITORY https://git.sr.ht/~redstrate/libphysis
        GIT_TAG        main
)

FetchContent_MakeAvailable(libphysis)

corrosion_import_crate(MANIFEST_PATH /home/josh/Development/libphysis/Cargo.toml)

target_include_directories(physis INTERFACE /home/josh/Development/libphysis/target/public)
target_link_libraries(physis INTERFACE unshield)