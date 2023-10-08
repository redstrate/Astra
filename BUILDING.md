# Building Astra

## Dependencies

### Required

All of these packages are required for Astra to build with a minimal set of features.

* Linux
  * Windows, macOS and other systems may work but are currently unsupported. Patches are accepted to fix any problems with those OSes though.
* CMake 3.25 or later
* Qt 6.5 or later
  * Modules: Base, Declarative, WebView
* KDE Frameworks 6
  * Modules: Extra CMake Modules, Kirigami, I18n, Config, and CoreAddons.
  * These are not packaged yet for any distributions, and won't be for a few months. See below for tips on where to source these.
* Rust
* Corrosion
* unshield
* QtKeychain for Qt6
* QuaZip for Qt6

### Optional

These are optional dependencies, that will be used if found on your system.

* Steamworks SDK
  * You must specify `STEAMWORKS_INCLUDE_DIRS` and `STEAMWORKS_LIBRARIES` yourself.
* Gamemode

## KDE Frameworks 6

Since KF6 is not going to be released for another few months minimum, you might have trouble sourcing the libraries.

### Arch Linux

You have the set of -git packages for the dependencies used, such as `kirigami2-git`. Otherwise, build from source.

### Fedora Linux

Fedora's KDE SIG has a repository for KDE Frameworks and Plasma 6 [located here](https://copr.fedorainfracloud.org/coprs/g/kdesig/kde-nightly-qt6/). I use this in my own personal [COPR](https://copr.fedorainfracloud.org/coprs/redstrate/personal/) which builds Astra from git.

### Build from source

The most time-consuming option but the one that will definitely work is building the required frameworks from source. See the [page on the KDE Community Wiki](https://community.kde.org/Get_Involved/development/Build_software_with_kdesrc-build) on how to build KDE software yourself. Then, when building Astra feed the kdesrc-build usr directory, and it will pick up on these libraries automatically: `-DCMAKE_PREFIX_PATH=/home/<username>/kde/usr`.

Remember that unless you're running in a kdesrc-build session you need to set some environment variables otherwise Astra will fail to run (because it fails to find some runtime KF6 dependencies). Make sure to `source prefix.sh` from the build directory, or from another kdesrc-build project.

## Configuring

**Note:** Some dependencies will automatically be downloaded from the Internet if not found on your system. This functionality may change in the future.

When configuring Astra, there are several optional features you may want to enable or disable:

* `ENABLE_STEAM`: Steam integration, requires the Steamworks SDK.
* `ENABLE_GAMEMODE`: Gamemode integration, requires Gamemode.

To configure, run `cmake` in the source directory:

```bash
$ cd astra
$ cmake -S . -B build
```

This command will create a new build directory and configure the source directory (`.`). If you want to enable more options, pass them now:

```bash
$ cmake -S . -B build -DENABLE_STEAM=ON
```

## Building

Now begin building the project:

```bash
$ cmake --build build
```

If the build was successful, an `astra` binary will be found in `./build/bin/astra`.
