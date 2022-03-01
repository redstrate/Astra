# Astra

[![sourcehut](https://img.shields.io/badge/repository-sourcehut-lightgrey.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSINCiAgICB3aWR0aD0iMTI4IiBoZWlnaHQ9IjEyOCI+DQogIDxkZWZzPg0KICAgIDxmaWx0ZXIgaWQ9InNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSJibGFjayIgLz4NCiAgICA8L2ZpbHRlcj4NCiAgICA8ZmlsdGVyIGlkPSJ0ZXh0LXNoYWRvdyIgeD0iLTEwJSIgeT0iLTEwJSIgd2lkdGg9IjEyNSUiIGhlaWdodD0iMTI1JSI+DQogICAgICA8ZmVEcm9wU2hhZG93IGR4PSIwIiBkeT0iMCIgc3RkRGV2aWF0aW9uPSIxLjUiDQogICAgICAgIGZsb29kLWNvbG9yPSIjQUFBIiAvPg0KICAgIDwvZmlsdGVyPg0KICA8L2RlZnM+DQogIDxjaXJjbGUgY3g9IjUwJSIgY3k9IjUwJSIgcj0iMzglIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjQlIg0KICAgIGZpbGw9Im5vbmUiIGZpbHRlcj0idXJsKCNzaGFkb3cpIiAvPg0KICA8Y2lyY2xlIGN4PSI1MCUiIGN5PSI1MCUiIHI9IjM4JSIgc3Ryb2tlPSJ3aGl0ZSIgc3Ryb2tlLXdpZHRoPSI0JSINCiAgICBmaWxsPSJub25lIiBmaWx0ZXI9InVybCgjc2hhZG93KSIgLz4NCjwvc3ZnPg0KCg==)](https://git.sr.ht/~redstrate/astra)
[![GitHub
mirror](https://img.shields.io/badge/mirror-GitHub-black.svg?logo=github)](https://github.com/redstrate/astra)

Astra is a FFXIV launcher that supports
**multiple profiles**, **Dalamud mods**, _and_ **macOS and Linux**!

At the moment, there are a few caveats that are good to know if you use these features:
* There is no **game patching** support, but the launcher will prompt you to launch another launcher anyway.
* There is no **news list** yet.
* The **Steam support is untested.**

Despite this, the launcher is still usable and I use it myself on a regular basis on Linux. I suggest you read more in the [FAQ](https://man.sr.ht/~redstrate/astra/faq.md)!

![screenshot](misc/screenshot.webp?raw=true)

## Features
* Can use native (Windows) and Wine-based (macOS, Linux) versions of FFXIV.
* You can use Dalamud, which is downloaded within the launcher just like XIVQuickLauncher. 
* Can connect to the official Square Enix servers _as well_ as Sapphire servers.
* Multiple profile support!
  * For example, you can have a regular Square Enix profile and a testing Sapphire profile. Or maybe one for DX11 and another for DX9?
  * These also have seperate, saved logins.
  * All settings can be configured per-profile! 
* Securely saving username and/or password. These are saved per-profile, and are encrypted using your system wallet, and will never be stored unencrypted.
* Encrypted game argument support similiar to what XIVQuickLauncher and the official ffxivboot does, preventing other programs from snooping your login token.
* Enable several (Linux) Wine-specific performance enhancements such as enabling Esync.
* On Linux, take advantafe of Watchdog to help notify you if you've moved in the login queue.
  * Only works on X11.
  * Will send you a notification on any change in the login queue. (moving up, logged in, lobby error, etc)
  * You can view your spot in the queue easily by using the system tray icon.

## Usage
There's lots of information located on [the wiki](https://man.sr.ht/~redstrate/astra/). However, there are several pages dedicated to using Astra:

* On [Windows](https://man.sr.ht/~redstrate/astra/windows-usage.md)
* On [macOS](https://man.sr.ht/~redstrate/astra/macos-usage.md)
* On [Linux](https://man.sr.ht/~redstrate/astra/linux-usage.md)

## Contributing and Support
Astra is primarily hosted at **sourcehut**, with the project located [here](https://sr.ht/~redstrate/astra/). There is multiple
ways to contribute patches:
* You can send a PR through the [Github mirror](https://github.com/redstrate/astra/pulls).
* If you have a sourcehut account, you can use the [web contributing interface](https://git.sr.ht/~redstrate/astra/send-email).
* If you refuse to use Github or sourcehut, you may always send me an email on [the astra mailing list](https://lists.sr.ht/~redstrate/astra-dev).

If you do send a patch through the mailing list, please prefix the subject with PATCH so it's properly filtered.

If you wish to **report an issue or discuss something**, the [the same astra-dev mailing list](https://lists.sr.ht/~redstrate/astra-dev) for now.