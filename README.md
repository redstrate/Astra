# Astra

A custom FFXIV launcher that supports multiple accounts, [Dalamud](https://github.com/goatcorp/Dalamud) plugins and runs
natively on Linux!

### Notice

Astra is _beta software_. Despite this, the launcher is feature complete, but you may find it lacking for the use-cases
below:

* [Logging into Steam-linked accounts is unsupported.](https://todo.sr.ht/~redstrate/astra/1)

![Main Screenshot](misc/screenshot.png)

If you still have questions, please read the [FAQ](https://xiv.zone/astra/faq) first.

## Features

* Handles running Wine for you, creating a seamless and native-feeling launcher experience!
    * Can also easily enable several Linux-specific enhancements such as Fsync or configuring Gamescope.
* Multiple account support!
    * Can associate a Lodestone character with an account to use as an avatar.
* Easily install and use Dalamud plugins.
* Game patching support.
* Securely login to the official Square Enix lobbies, as well as Sapphire servers.
    * Game arguments are encrypted by default, providing the same level of security as other launchers.
    * Saving account usernames and passwords are also supported, and is never stored plaintext.
* Can install FFXIV on new systems for you, bypassing the normal InstallShield installer.

## Installation

Precompiled binaries are available [to download from the website](https://xiv.zone/astra/install).

There are also numerous options available:

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
[important usage information](https://man.sr.ht/~redstrate/astra/linux-usage.md).

## Contributing and Support

The best way you can help Astra is by [monetarily supporting me](https://ko-fi.com/redstrate) or by submitting patches
to fix bugs or add functionality.
I work on Astra and my other FFXIV projects in my free time, so any support helps to let me continue what I do!

If you have changes you wish to submit, you can use [git send-email](https://git-send-email.io/) or
the [sourcehut web contributor interface](https://git.sr.ht/~redstrate/astra/send-email)!

If you wish to file a bug report, or feature request please see [the bug tracker](https://todo.sr.ht/~redstrate/astra).

For general discussion about the launcher, potential features and so on, send an [email through the mailing list](https://lists.sr.ht/~redstrate/public-inbox).
I discourage contacting me privately unless necessary, so that everyone can benefit from the discussion.