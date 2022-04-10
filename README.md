# Astra

[![sourcehut](https://img.shields.io/badge/repository-sourcehut-lightgrey.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSINCiAgICB3aWR0aD0iMTI4IiBoZWlnaHQ9IjEyOCI+DQogIDxkZWZzPg0KICAgIDxmaWx0ZXIgaWQ9InNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSJibGFjayIgLz4NCiAgICA8L2ZpbHRlcj4NCiAgICA8ZmlsdGVyIGlkPSJ0ZXh0LXNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSIjQUFBIiAvPg0KICAgIDwvZmlsdGVyPg0KICA8L2RlZnM+DQogIDxjaXJjbGUgY3g9IjUwJSIgY3k9IjUwJSIgcj0iMzglIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjQlIg0KICAgIGZpbGw9Im5vbmUiIGZpbHRlcj0idXJsKCNzaGFkb3cpIiAvPg0KICA8Y2lyY2xlIGN4PSI1MCUiIGN5PSI1MCUiIHI9IjM4JSIgc3Ryb2tlPSJ3aGl0ZSIgc3Ryb2tlLXdpZHRoPSI0JSINCiAgICBmaWxsPSJub25lIiBmaWx0ZXI9InVybCgjc2hhZG93KSIgLz4NCjwvc3ZnPg0KCg==)](https://git.sr.ht/~redstrate/astra)
[![GitHub
mirror](https://img.shields.io/badge/mirror-GitHub-black.svg?logo=github)](https://github.com/redstrate/astra)
[![ryne.moe
mirror](https://img.shields.io/badge/mirror-ryne.moe-red.svg?logo=git)](https://git.ryne.moe/redstrate/astra)

A FFXIV launcher that supports **multiple profiles** and **Dalamud plugins**. It also supports Windows, macOS and Linux natively!

### Beta Notice
Astra is **beta software**. At the moment, there are a few caveats that are good to know if you use these features:
* There is no **game patching** support, but the launcher will still prompt you to update.
* The **Steam support is untested.**

Despite this, the launcher is still usable, and I use it myself on a regular basis on Linux. If you
have more questions, I suggest reading the [FAQ](https://man.sr.ht/~redstrate/astra/faq.md).

![screenshot](misc/screenshot.webp?raw=true)

## Features
* Can **bootstrap a new FFXIV installation** if it can't find one. You can skip the installer entirely!
* Can use **native (Windows)** and **Wine-based (macOS, Linux)** versions of FFXIV.
* You can use **Dalamud plugins**, which is downloaded within the launcher just like XIVQuickLauncher. 
* Can connect to the **official Square Enix servers** _as well_ as **Sapphire servers**.
* **Multiple profiles**!
  * For example, you can have a regular Square Enix profile and a testing Sapphire profile. Or maybe one for DX11 and another for DX9?
  * These also have seperate, saved logins.
  * All settings can be configured per-profile! 
* **Securely saving username and/or password**. These are saved per-profile, and are encrypted using your system wallet, and will never be stored unencrypted.
* **Encrypted game argument** support similiar to what XIVQuickLauncher and the official ffxivboot does, preventing other programs from snooping your login token.
* Enable several Linux-specific Wine performance enhancements such as **enabling Fsync/Esync/Futex2**.

## Installation
Pre-compiled binaries are not yet available. However if you use Arch Linux, there is a PKGBUILD available in the AUR 
for [tagged releases](https://aur.archlinux.org/packages/astra-launcher) and [straight from git](https://aur.archlinux.org/packages/astra-launcher-git). You may install it through `makepkg` or your favorite AUR helper:

```
$ aur sync astra-launcher
```


## Usage
There's lots of information located on [the wiki](https://man.sr.ht/~redstrate/astra/)! Here's pages dedicated to
using/building Astra on it's supported platforms:

* On [Windows](https://man.sr.ht/~redstrate/astra/windows-usage.md)
* On [macOS](https://man.sr.ht/~redstrate/astra/macos-usage.md)
* On [Linux](https://man.sr.ht/~redstrate/astra/linux-usage.md)

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