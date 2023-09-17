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

    QCoro::Task<> checkGateStatus(const LoginInformation &info);

private:
    QCoro::Task<> bootCheck(const LoginInformation &info);

    Patcher *patcher = nullptr;

    LauncherCore &window;
    SquareLauncher &launcher;
};