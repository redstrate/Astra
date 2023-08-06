#!/bin/sh

flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo &&
flatpak install --user -y runtime/org.kde.Platform/x86_64/5.15-22.08 runtime/org.kde.Sdk/x86_64/5.15-22.08 org.freedesktop.Sdk.Extension.rust-stable/x86_64/22.08 &&
flatpak-builder build --user --force-clean zone.xiv.astra.yml &&
flatpak build-export export build &&
flatpak build-bundle export astra.flatpak zone.xiv.astra --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo