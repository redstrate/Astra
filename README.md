# Astra

Astra is a FFXIV launcher that supports
**multiple profiles**, **Dalamud mods**, _and_ **macOS and Linux**!

At the moment, there are a few caveats that are good to know if you use these features:
* There is no **game patching** support, but the launcher will prompt you to launch another launcher anyway.
* There is no **news list** yet.
* The **Steam support is untested.**

Despite this, the launcher is still usable and I use it myself on a regular basis on Linux. I suggest you read more in the [FAQ](https://github.com/redstrate/astra/wiki/Frequently-Asked-Questions)!

![screenshot](https://github.com/redstrate/astra/blob/main/misc/screenshot.webp?raw=true)

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
* On [Windows](https://github.com/redstrate/astra/wiki/Windows-Usage)
* On [macOS](https://github.com/redstrate/astra/wiki/macOS-Usage)
* On [Linux](https://github.com/redstrate/astra/wiki/Linux-Usage)
