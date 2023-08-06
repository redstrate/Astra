// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "launchercore.h"
#include "patcher.h"

class SquareLauncher : public QObject
{
    Q_OBJECT
public:
    explicit SquareLauncher(LauncherCore &window, QObject *parent = nullptr);

    void getStored(const LoginInformation &info);

    void login(const LoginInformation &info, const QUrl &referer);

    void registerSession(const LoginInformation &info);

private:
    QString getBootHash(const LoginInformation &info);

    Patcher *patcher = nullptr;

    QString stored, SID, username;
    LoginAuth auth;

    LauncherCore &window;
};
