#!/bin/sh

# ensure the submodules are up to date
git submodule init
git submodule update

# begin vendoring cargo dependencies
cd external/libphysis
cargo vendor ../../cargo-vendored
# workaround for https://github.com/rust-lang/cargo/issues/7058, winapi libraries take up a ton of space
rm -fr ../../cargo-vendored/winapi*gnu*
rm -fr ../../cargo-vendored/windows*
mkdir .cargo
cp ../../scripts/config.toml .cargo/config.toml
cd ../../

tar --exclude='cmake-build*' --exclude='.idea' --exclude='.clang-format' --exclude='astra-source.tar.gz' --exclude-vcs -zcvf ../astra-source.tar.gz .
