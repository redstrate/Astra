// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "patcher.h"

class SquareLauncher;
class LauncherCore;
class LoginInformation;

class SquareBoot : public QObject
{
    Q_OBJECT

public:
    SquareBoot(LauncherCore &window, SquareLauncher &launcher, QObject *parent = nullptr);

    QCoro::Task<> checkGateStatus(LoginInformation *info);

    QCoro::Task<> bootCheck(const LoginInformation &info);

private:
    Patcher *patcher = nullptr;

    LauncherCore &window;
    SquareLauncher &launcher;
};