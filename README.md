# xivlauncher

Finally, a cross-platform FFXIV launcher. It should run on Windows, macOS and Linux!

Compared to XIVQuickLauncher and the official launcher, this supports
**multiple profiles**, **Dalamud mods**, _and_ **macOS and Linux**! This means you no longer
have to suffer running your FFXIV launcher through Wine. Under Linux, you have **Watchdog** available to help you through the login queue.

As of this moment, the three missing major features are **game patching**, **external tool launching** and **the news list**.
If you don't use these features then the launcher is still usable.

More information can be found in the [FAQ](https://github.com/redstrate/xivlauncher/wiki/Frequently-Asked-Questions).

![screenshot](https://github.com/redstrate/xivlauncher/blob/main/misc/screenshot.png?raw=true)

## Features
* Can use native (Windows) and Wine (macOS, Linux) versions of FFXIV.
* You can use Dalamud, which is downloaded within the launcher just like XIVQuickLauncher. 
* Can connect to the official Square Enix servers _as well_ as Sapphire servers.
* Multiple profile support!
  * For example, having a regular Square Enix profile and a test Sapphire profile. Or maybe one for DX11 and another for DX9?
  * All settings can be configured per-profile! 
* Saving username and/or password. These are saved per-profile, and are encrypted using your system wallet.
* Encrypted game argument support similiar to what XIVQuickLauncher and the official ffxivboot already does.
* Enable several (Linux) Wine-specific performance enhancements such as enabling Esync.
* You have a **Watchdog** available to help you through queues.
  * Only works on X11 and Linux (at the moment)
  * Will send you a notification on any change in the login queue. (moving up, logged in, lobby error, etc)
  * Can view your spot in the queue easily by using the system tray icon.

## Usage
Pre-compiled binaries are not (yet) available, so you must compile from source. Please see the relevant
usage guide for your platform:
* [Windows](https://github.com/redstrate/xivlauncher/wiki/Windows-Usage)
* [macOS](https://github.com/redstrate/xivlauncher/wiki/macOS-Usage)
* [Linux](https://github.com/redstrate/xivlauncher/wiki/Linux-Usage)
