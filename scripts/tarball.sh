#!/bin/sh

# ensure the submodules are up to date
git submodule init
git submodule update

# begin vendoring cargo dependencies
cd external/libphysis
cargo vendor ../../cargo-vendored
mkdir .cargo
cp ../../scripts/config.toml .cargo/config.toml
cd ../../

tar --exclude='*build*' --exclude='.idea' --exclude='.clang-format' --exclude='astra-source.tar.gz' --exclude-vcs -zcvf ../astra-source.tar.gz .
