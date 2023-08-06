// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "launchercore.h"

class SapphireLauncher : QObject
{
public:
    explicit SapphireLauncher(LauncherCore &window, QObject *parent = nullptr);

    void login(const QString &lobbyUrl, const LoginInformation &info);
    void registerAccount(const QString &lobbyUrl, const LoginInformation &info);

private:
    LauncherCore &window;
};