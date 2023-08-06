// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include "launchercore.h"

#if defined(Q_OS_LINUX)
#include "gameparser.h"
#endif

#include <QSystemTrayIcon>

class Watchdog : public QObject
{
    Q_OBJECT
public:
    Watchdog(LauncherCore &core)
        : core(core)
        , QObject(&core)
    {
    }

    void launchGame(const ProfileSettings &settings, const LoginAuth &auth);

private:
    LauncherCore &core;
    QSystemTrayIcon *icon = nullptr;

    int processWindowId = -1;

#if defined(Q_OS_LINUX)
    GameParseResult lastResult;
#endif

    std::unique_ptr<GameParser> parser;
};