# Contributing to Astra

I appreciate any code contributions to Astra, whether they are bugfixes or new features! this repository is managed on [sourcehut](https://git.sr.ht/~redstrate/astra).

Once you finished your changes, use [git send-email](https://git-send-email.io/) or the [sourcehut web contributor interface](https://git.sr.ht/~redstrate/astra/send-email) to upload them. Please send your patches to [~redstrate/public-inbox@lists.sr.ht](mailto:~redstrate/public-inbox@lists.sr.ht).

## What Needs Doing

The current TODO list can be located [here](https://todo.sr.ht/~redstrate/astra). It's the best place for feature requests and bug reports.

## Where Stuff Is

Everything code-wise for Astra in the [/launcher](launcher) directory. Astra follows modern Kirigami and Qt practices, meaning the frontend is written in QML and the backend is C++.

### Backend

The backend code is written in C++, and we're currently requiring C++20 to build.

### Frontend

The user interface is written in QML, using technologies such as Kirigami. Please see the [KDE API Documentation](https://api.kde.org/) for more information. The frontend files are located in [/launcher/ui](/launcher/ui).

## Conventions

Astra follows the standard KDE coding style, which is enforced by the use of [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and a git hook. There is nothing else you need to do, please commit any fixes the tool makes.