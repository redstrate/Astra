# Astra

[![sourcehut](https://img.shields.io/badge/repository-sourcehut-lightgrey.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSINCiAgICB3aWR0aD0iMTI4IiBoZWlnaHQ9IjEyOCI+DQogIDxkZWZzPg0KICAgIDxmaWx0ZXIgaWQ9InNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSJibGFjayIgLz4NCiAgICA8L2ZpbHRlcj4NCiAgICA8ZmlsdGVyIGlkPSJ0ZXh0LXNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSIjQUFBIiAvPg0KICAgIDwvZmlsdGVyPg0KICA8L2RlZnM+DQogIDxjaXJjbGUgY3g9IjUwJSIgY3k9IjUwJSIgcj0iMzglIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjQlIg0KICAgIGZpbGw9Im5vbmUiIGZpbHRlcj0idXJsKCNzaGFkb3cpIiAvPg0KICA8Y2lyY2xlIGN4PSI1MCUiIGN5PSI1MCUiIHI9IjM4JSIgc3Ryb2tlPSJ3aGl0ZSIgc3Ryb2tlLXdpZHRoPSI0JSINCiAgICBmaWxsPSJub25lIiBmaWx0ZXI9InVybCgjc2hhZG93KSIgLz4NCjwvc3ZnPg0KCg==)](https://sr.ht/~redstrate/astra)
[![GitHub
mirror](https://img.shields.io/badge/mirror-GitHub-black.svg?logo=github)](https://github.com/redstrate/astra)
[![ryne.moe
mirror](https://img.shields.io/badge/mirror-ryne.moe-red.svg?logo=git)](https://git.ryne.moe/redstrate/astra)

A custom FFXIV launcher that supports **multiple accounts/profiles**, **Dalamud plugins** and **runs natively** on
Windows, macOS and Linux!

### Beta Notice
Astra is **beta software**. Despite this, the launcher is usable for most usecases, except for those noted below:

* Astra **cannot update the game** on its own yet, however you'll be prompted to use the official launcher to update.
* Logging into **Steam-linked accounts** is unsupported at the moment.

If you have more questions, please look at the [FAQ](https://man.sr.ht/~redstrate/astra/faq.md).

![Main Screenshot](misc/screenshot.png)

## Features
* **Native versions of the launcher** available for Windows, macOS and Linux. Don't run your launcher through Wine anymore! 
* Can **install FFXIV** for you if you're on a new computer.
* Can run FFXIV **natively (Windows)** or through **Wine-based methods (macOS, Linux)**.
* Easily integrate **Dalamud plugins**, just like XIVQuickLauncher. 
* Can connect to the **official Square Enix servers** _as well_ as **Sapphire servers**, replacing the need for a whole different launcher.
* **Multiple profiles** with their own account credentials!
  * All settings can be configured per-profile! 
* **Save your username and password**! These are encrypted using your system wallet, and will never be stored unencrypted.
* **Encrypted game arguments** enabled by default, providing the same level of security as the official launcher. 
* Enable several Linux-specific Wine performance enhancements such as **enabling Fsync/Esync/Futex2** or configuring Gamescope!

## Installation
There are precompiled binaries available through the [Install page](https://xiv.zone/astra/install) on the official website.

### Windows

Simply extract the zip file into any directory, and run `Astra.exe`. Your user data, Dalamud plugin configs, and passwords
are not stored in the program directory.

### macOS

Move `Astra.app` into your `/Applications` or `~/Applications` folder and run it. The official macOS builds are
notarized under my Apple Developer license, so they should work even with gatekeeper enabled.

In order to launch FFXIV, you must have a version of Wine installed on your system. You can install it from homebrew, use it
from the official FFXIV Mac version, or use XIV on Mac's wine. Just select your preferred "Wine Type" in your profile settings.

### Linux

#### Flatpak

There is a Flatpak app available from the xiv.zone Flatpak repository. Follow the [Flatpak instructions](https://xiv.zone/install#Flatpak)
to learn how to add it and download Astra.

#### AUR

I maintain the Astra PKGBUILDS, available from the AUR for [tagged releases](https://aur.archlinux.org/packages/astra-launcher) or [straight from git](https://aur.archlinux.org/packages/astra-launcher-git). You may install it through `makepkg` or your favorite AUR helper:

```
$ aur sync astra-launcher
```


## Building
There's lots of information located on [the wiki](https://man.sr.ht/~redstrate/astra/)! Here's pages dedicated to
building Astra on it's supported platforms:

* [Windows](https://man.sr.ht/~redstrate/astra/windows-usage.md)
* [macOS](https://man.sr.ht/~redstrate/astra/macos-usage.md)
* [Linux](https://man.sr.ht/~redstrate/astra/linux-usage.md)

## Contributing and Support
Astra is primarily hosted at **sourcehut**, with the project located [here](https://sr.ht/~redstrate/astra/). There is multiple
ways to contribute patches:
* You can send a PR through the [Github mirror](https://github.com/redstrate/astra/pulls).
* If you have a sourcehut account, you can use the [web contributing interface](https://git.sr.ht/~redstrate/astra/send-email).
* If you do not want to use Github or sourcehut, you may always send the patch through [the mailing list](https://lists.sr.ht/~redstrate/astra-dev).

If you do send a patch through the mailing list, please prefix the subject with PATCH so it's properly filtered.

If you wish to **report an issue or discuss something**, also use [the mailing list](https://lists.sr.ht/~redstrate/astra-dev) for now.
I highly encourage you to send an email through the mailing list instead of contacting me privately if possible, so
other people may benefit from the solution.