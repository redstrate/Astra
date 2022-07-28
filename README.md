# Astra

A custom FFXIV launcher that supports **multiple accounts/profiles**, **Dalamud plugins** and **runs natively** on
Windows, macOS and Linux!

### Beta Notice
Astra is **beta software**. Despite this, the launcher is usable for most use-cases, except for those noted below:

* Logging into **Steam-linked accounts** is unsupported at the moment.

If you have more questions, please look at the [FAQ](https://xiv.zone/astra/faq).

![Main Screenshot](misc/screenshot.png)

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
* Can patch the game natively, avoiding having to boot the official launcher to do so.
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

If you have changes you wish to submit, you can use [git send-email](https://git-send-email.io/) or the [sourcehut web contributor interface](https://git.sr.ht/~redstrate/astra/send-email)! 

If you wish to file a bug report, or feature request please see [the bug tracker](https://todo.sr.ht/~redstrate/astra).

For general discussion about the launcher, potential features and so on, email me [on my mailing list](https://lists.sr.ht/~redstrate/public-inbox).
I discourage contacting me privately unless necessary, so everyone can benefit from the discussion.