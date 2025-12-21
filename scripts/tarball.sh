#!/bin/sh

# ensure the submodules are up to date
git submodule init &&
git submodule update &&

tar --exclude='cmake-build*' --exclude='.idea' --exclude='.clang-format' --exclude='astra-source.tar.gz' --exclude='.flatpak-builder' --exclude='export' --exclude='build' --exclude='.cache' --exclude='compile_commands.json' --exclude='.kateproject.build' --exclude='.directory' --exclude='*.flatpak' --exclude-vcs -zcvf ../astra-source.tar.gz .
