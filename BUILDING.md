# Building Astra

## Dependencies

### Required

All of these packages are required for Astra to build with a minimal set of features.

* Linux
  * Windows, macOS and other systems may work but are currently unsupported.
* CMake 3.16 or later
* Qt 5.15 or later
* KDE Frameworks 5.83 or later
  * Extra CMake Modules, Kirigami, I18n, Config, and CoreAddons.
* Rust
* unshield
* QtKeychain 5
* QuaZip 5

### Optional

* Steamworks SDK
  * You must specify `STEAMWORKS_INCLUDE_DIRS` and `STEAMWORKS_LIBRARIES` yourself.
* Gamemode
* Tesseract

## Configuring

**Note:** Some dependencies will automatically be downloaded from the Internet if not found on your system. This functionality may change in the future.

When configuring Astra, there are several optional features you may want to enable or disable:

* `ENABLE_WATCHDOG`: Watchdog support, requires Tesseract and X11.
* `ENABLE_STEAM`: Steam integration, requires the Steamworks SDK.
* `ENABLE_GAMEMODE`: Gamemode integration, reqires Gamemode.

To configure, run `cmake` in the source directory:

```bash
$ cd astra
$ cmake -S . -B build
```

This command will create a new build directory and configure the source directory (`.`). If you want to enable more options, pass the mnow:

```bash
$ cmake -S . -B build -DENABLE_STEAM=ON
```

## Building

Now begin building the project:

```bash
$ cmake --build build
```

If the build was successful, an `astra` binary will be found in `./build/bin/astra`.