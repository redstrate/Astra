# Astra

[![sourcehut](https://img.shields.io/badge/repository-sourcehut-lightgrey.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSINCiAgICB3aWR0aD0iMTI4IiBoZWlnaHQ9IjEyOCI+DQogIDxkZWZzPg0KICAgIDxmaWx0ZXIgaWQ9InNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSJibGFjayIgLz4NCiAgICA8L2ZpbHRlcj4NCiAgICA8ZmlsdGVyIGlkPSJ0ZXh0LXNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSIjQUFBIiAvPg0KICAgIDwvZmlsdGVyPg0KICA8L2RlZnM+DQogIDxjaXJjbGUgY3g9IjUwJSIgY3k9IjUwJSIgcj0iMzglIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjQlIg0KICAgIGZpbGw9Im5vbmUiIGZpbHRlcj0idXJsKCNzaGFkb3cpIiAvPg0KICA8Y2lyY2xlIGN4PSI1MCUiIGN5PSI1MCUiIHI9IjM4JSIgc3Ryb2tlPSJ3aGl0ZSIgc3Ryb2tlLXdpZHRoPSI0JSINCiAgICBmaWxsPSJub25lIiBmaWx0ZXI9InVybCgjc2hhZG93KSIgLz4NCjwvc3ZnPg0KCg==)](https://sr.ht/~redstrate/astra)
[![GitHub
mirror](https://img.shields.io/badge/mirror-GitHub-black.svg?logo=github)](https://github.com/redstrate/astra)
[![ryne.moe
mirror](https://img.shields.io/badge/mirror-ryne.moe-red.svg?logo=git)](https://git.ryne.moe/redstrate/astra)

A custom FFXIV launcher that supports multiple accounts, Dalamud plugins, and Windows/macOS/Linux. Astra is based on Qt,
which provides a native-feeling GUI but also lean on resources.

![Main Screenshot](misc/screenshot.png)

### Beta Notice
Astra is currently in **beta**. The launcher is usable right now, and I even use this as my sole way to play FFXIV.
There are a few caveats that are good to know beforehand though:

* Astra cannot update the game on its own yet, however you'll be prompted to use the official launcher to update.
* Logging into Steam-linked accounts is unsupported at the moment.

If you have more questions, please read at the [FAQ](https://xiv.zone/astra/faq) first.

## Features
* Traditional desktop interface which looks native to your system, utilizing Qt - a proven application framework.
  * A Tablet/TV interface designed for touchscreens or handhelds such as the Steam Deck is also available.
  * Can even run without a GUI, ideal for users comfortable with a CLI or for automation.
* Native support for Windows, macOS and Linux!
  * An official Flatpak is also available for Linux distributions which do not yet have a package, or for users who prefer it.
* Handles running Wine for macOS and Linux users - creating a seamless and native-feeling launcher experience, compared to running other FFXIV launchers in Wine.
  * Can also easily enable several Linux-specific enhancements such as Fsync or configuring Gamescope.
* Multiple accounts support!
  * Almost all settings (including different game installs) can also be set per-profile.
* Easily install and use Dalamud plugins, just like with XIVQuickLauncher. 
* Securely login to the official Square Enix lobbies, as well as Sapphire servers.
  * Game arguments are encrypted by default, providing the same level of security as other launchers.
  * Saving account usernames and passwords are also supported, and is never stored plaintext.
* Can easily install FFXIV on new systems right from the launcher, bypassing the normal InstallShield installer.

## Installation
Precompiled binaries for Windows and macOS are available on [here](https://xiv.zone/astra/install).

For Linux users, there is a Flatpak available (although right now it's located in the xiv.dev Flatpak repository, work is ongoing to submit this to Flathub.)
Instructions can be found in the [Flatpak installation](https://xiv.zone/astra/install/#linux) section.

If you don't prefer Flatpak, I maintain [the AUR version](https://aur.archlinux.org/packages/astra-launcher) for Arch
Linux users. Distribution packaging is encouraged, so please send an [email on my mailing list](https://lists.sr.ht/~redstrate/public-inbox) for any concerns.

## Building
**Note:** Some dependencies will automatically be downloaded from the Internet if not found  on your system. 
This functionality will change in the future to ease distribution packaging. You can control this functionality using
the `USE_OWN_LIBRARIES` CMake option.

[The wiki](https://man.sr.ht/~redstrate/astra/) has dedicated platform-specific pages for build instructions as well as important information:

* [Windows](https://man.sr.ht/~redstrate/astra/windows-usage.md)
* [macOS](https://man.sr.ht/~redstrate/astra/macos-usage.md)
* [Linux](https://man.sr.ht/~redstrate/astra/linux-usage.md)

## Contributing and Support
The best way you can help Astra is by [monetarily supporting me](https://ko-fi.com/redstrate) or by submitting patches to fix bugs or add functionality.
I work on Astra and my other FFXIV projects in my free time, so any support helps to let me  continue what I do!

Astra is [lives on sourcehut](https://sr.ht/~redstrate/astra/). There are two ways you can contribute patches:
* If you prefer GitHub, I accept pull requests on the  [Github mirror](https://github.com/redstrate/astra/pulls).
* You can send patches by sending an email to [my mailing list](https://lists.sr.ht/~redstrate/public-inbox). If you do send a patch through the mailing list, please prefix the subject with `[PATCH astra]` so it's properly filtered.

If you just want to report a bug or discuss a feature, please send an email to [my mailing list](https://lists.sr.ht/~redstrate/public-inbox).
I discourage contacting me privately unless necessary, because other people may have the same problem as you - and I can share the solution with everyone :-)