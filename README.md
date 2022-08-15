# Astra

A custom FFXIV launcher that supports multiple accounts, [Dalamud](https://github.com/goatcorp/Dalamud) plugins and runs
natively on  Windows, macOS and Linux!

### Notice

Astra is _beta software_. Despite this, the launcher is feature complete, but you may find it lacking for the use-cases
below:

* [Logging into Steam-linked accounts is unsupported.](https://todo.sr.ht/~redstrate/astra/1)

![Main Screenshot](misc/screenshot.png)

If you still have questions, please read the [FAQ](https://xiv.zone/astra/faq) first.

## Features

* Traditional desktop interface which looks native to your system, utilizing Qt - a proven application framework.
    * A Tablet/TV interface designed for touchscreens or handhelds such as the Steam Deck is also available.
    * Can even run without a GUI, ideal for users comfortable with a CLI or for automation.
* Native support for Windows, macOS and Linux!
* Handles running Wine for macOS and Linux users - creating a seamless and native-feeling launcher experience, compared
  to running other FFXIV launchers in Wine.
    * Can also easily enable several Linux-specific enhancements such as Fsync or configuring Gamescope.
* Multiple account support!
    * Most settings can be set per-profile.
* Easily install and use Dalamud plugins, just like XIVQuickLauncher.
* Patches the game, just like the official launcher!
* Securely login to the official Square Enix lobbies, as well as Sapphire servers.
    * Game arguments are encrypted by default, providing the same level of security as other launchers.
    * Saving account usernames and passwords are also supported, and is never stored plaintext.
* Can easily install FFXIV on new systems right from the launcher, bypassing the normal InstallShield installer.

## Installation

Precompiled binaries are available for Windows and macOS users, which you can [download from the website](https://xiv.zone/astra/install).

For Linux users, there is numerous options available to you:

* _Flatpak_ - Instructions can be found in the [Flatpak installation](https://xiv.zone/astra/install/#linux) section.
* _AUR_ - You can find the [AUR package here](https://aur.archlinux.org/packages/astra-launcher).
* _Gentoo Overlay_ - You can find a Gentoo ebuild in my [personal overlay](https://git.sr.ht/~redstrate/ebuilds/tree/master/item/games-misc/astra).
* _Tarball_ - You can download the latest release's tarball from [the website](https://xiv.zone/astra/install).

Distribution packaging is encouraged, so please send
an [email through the mailing list](https://lists.sr.ht/~redstrate/public-inbox) for any concerns.

## Building

**Note:** Some dependencies will automatically be downloaded from the Internet if not found on your system.
This functionality will change in the future to ease distribution packaging. You can control this functionality using
the `USE_OWN_LIBRARIES` CMake option.

[The wiki](https://man.sr.ht/~redstrate/astra/) has dedicated platform-specific pages for build instructions as well as
important information:

* [Windows](https://man.sr.ht/~redstrate/astra/windows-usage.md)
* [macOS](https://man.sr.ht/~redstrate/astra/macos-usage.md)
* [Linux](https://man.sr.ht/~redstrate/astra/linux-usage.md)

## Contributing and Support

The best way you can help Astra is by [monetarily supporting me](https://ko-fi.com/redstrate) or by submitting patches
to fix bugs or add functionality.
I work on Astra and my other FFXIV projects in my free time, so any support helps to let me continue what I do!

If you have changes you wish to submit, you can use [git send-email](https://git-send-email.io/) or
the [sourcehut web contributor interface](https://git.sr.ht/~redstrate/astra/send-email)!

If you wish to file a bug report, or feature request please see [the bug tracker](https://todo.sr.ht/~redstrate/astra).

For general discussion about the launcher, potential features and so on, send an [email through the mailing list](https://lists.sr.ht/~redstrate/public-inbox).
I discourage contacting me privately unless necessary, so that everyone can benefit from the discussion.