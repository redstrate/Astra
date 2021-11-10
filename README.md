# xivlauncher

Finally, a cross-platform FFXIV launcher. It should run on Windows, macOS and Linux!

Compared to XIVQuickLauncher and the official launcher, this supports
**multiple profiles** and **macOS and Linux**! This means you no longer
have to suffer running your launcher through Wine.

As of this moment, the three missing major features are **game patching** and **Dalamud and other external tool launching** and **the news list**.
If you don't use these features then the launcher is still usable.

![screenshot](https://github.com/redstrate/xivlauncher/blob/main/misc/screenshot.png?raw=true)

## Features
* Can use native (Windows) and Wine (macOS, Linux) versions of FFXIV.
* Can connect to the official Square Enix servers _as well_ as Sapphire servers.
* Multiple profile support!
  * For example, having a regular Square Enix profile and a test Sapphire profile. Or maybe one for DX11 and another for DX9?
  * All settings can be configured per-profile! 
* Saving username and/or password. These are saved per-profile, and are encrypted using your system wallet.
* Encrypted game argument support similiar to what XIVQuickLauncher and the official ffxivboot already does.
* Enable several (Linux) Wine-specific performance enhancements such as enabling Esync.

## Usage
Pre-compiled binaries are not (yet) available, so you must compile from source. Please see the relevant
usage guide for your platform:
* [Windows](https://github.com/redstrate/xivlauncher/wiki/Windows-Usage)
* [macOS](https://github.com/redstrate/xivlauncher/wiki/macOS-Usage)
* [Linux](https://github.com/redstrate/xivlauncher/wiki/Linux-Usage)
