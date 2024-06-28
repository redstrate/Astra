#!/bin/sh

# ensure the submodules are up to date
git submodule init &&
git submodule update &&

# begin vendoring cargo dependencies
# TODO: should we really do this? an error blocked 0.6.0 release
#cd external/libphysis &&
#cargo vendor-filterer --platform=x86_64-unknown-linux-gnu ../../cargo-vendored &&
#mkdir .cargo &&
#cp ../../scripts/config.toml .cargo/config.toml &&
#cd ../../ &&

tar --exclude='cmake-build*' --exclude='.idea' --exclude='.clang-format' --exclude='astra-source.tar.gz' --exclude-vcs -zcvf ../astra-source.tar.gz .
