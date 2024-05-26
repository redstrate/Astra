#!/bin/sh

flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo &&
flatpak-builder build --user --force-clean --install-deps-from=flathub zone.xiv.astra.yml --gpg-sign=0xD28B9141A3B3A73A --repo=/home/josh/sources/flatpak-distrib
