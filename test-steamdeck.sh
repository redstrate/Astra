#!/bin/sh

flatpak-builder build --force-clean zone.xiv.astra.yml &&
flatpak build-export export build &&
flatpak build-bundle export astra-next.flatpak zone.xiv.astra --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo &&
rsync ./astra-next.flatpak deck@steamdeck:~/astra-next.flatpak
ssh deck@steamdeck 'flatpak install --user --noninteractive -y astra-next.flatpak'
#ssh deck@steamdeck 'flatpak run --user zone.xiv.astra'