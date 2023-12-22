# Flatpak Build Instructions

This builds the Flatpak in a directory called `build` and a repository called `export`:

```shell
$ flatpak-builder --force-clean --user --install --repo=export build zone.xiv.astra.yml
```

Some other useful options I use:
* `--disable-updates`: Disable updates for VCS like git, useful for quick iteration on Astra itself.

To create a bundle, use this command:

```shell
$ flatpak build-bundle export astra.flatpak zone.xiv.astra  --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
```

This should create a bundle called `astra.flatpak`.