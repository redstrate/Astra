#!/bin/sh

# ensure the submodules are up to date
git submodule init &&
git submodule update &&

tar --exclude='cmake-build*' --exclude='.idea' --exclude='.clang-format' --exclude='astra-source.tar.gz' --exclude-vcs -zcvf ../astra-source.tar.gz .
