# Building Astra

There are two methods to build Astra, either via [Flatpak](https://flatpak.org/) or manually using your system libraries. It's highly recommended to prefer the Flatpak, especially if you don't have experience with CMake, C++ and such.

## Flatpak

Building the Flatpak version is easy, and there's a helper script to speed up the process. You must run it from the repository root:

```
$ cd astra
$ ./scripts/build-flatpak.sh
```

The process should only take a few minutes on a moderately powerful machine. It does require an Internet connection and the relevant permissions to install the required Flatpak runtimes and extensions.

When it's complete, a file called `astra.flatpak` will appear in the repository root and that can be installed with the `flatpak` CLI tool or your preferred application store.

## Manual

The process to build Astra manually is a little bit more involved, but not difficult. It's easiest to do on rolling release Linux distributions. 

### Dependencies

#### Required

* [Linux](https://kernel.org/)
  * Windows, macOS and other systems may work but are currently unsupported. Patches are accepted to fix any problems with those OSes though.
* [CMake](https://cmake.org) 3.25 or later
* [Qt](https://www.qt.io/) 6.6 or later
  * Base, Declarative, WebView, Concurrent
* [KDE Frameworks](https://develop.kde.org/products/frameworks/) 6
  * Extra CMake Modules, Kirigami, I18n, Config, CoreAddons and Archive.
* [Rust](https://www.rust-lang.org/)
* [Corrosion](https://github.com/corrosion-rs/corrosion)
* [unshield](https://github.com/twogood/unshield)
* [QtKeychain](https://github.com/frankosterfeld/qtkeychain)
* [QCoro](https://qcoro.dvratil.cz/)

#### Optional

These are optional dependencies, that will be used if found on your system.

* Steamworks SDK
  * You must specify `STEAMWORKS_INCLUDE_DIRS` and `STEAMWORKS_LIBRARIES` yourself.
* Gamemode
  * Turn on `ENABLE_GAMEMODE` to enable integration.

### Configuring

To configure, run `cmake` in the source directory:

```bash
$ cd astra
$ cmake -S . -B build
```

This command will create a new build directory and configure the source directory (`.`). If you want to enable more options, pass them now:

```bash
$ cmake -S . -B build -DENABLE_GAMEMODE=ON
```

## Building

Now begin building the project:

```bash
$ cmake --build build
```

If the build was successful, an `astra` binary will be found in `./build/bin/astra`.
