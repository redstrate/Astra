#pragma once

#include "patcher.h"

class SquareLauncher;
class LauncherCore;
class LoginInformation;

class SquareBoot : public QObject
{
    Q_OBJECT
public:
    SquareBoot(LauncherCore &window, SquareLauncher &launcher, QObject *parent = nullptr);

    Q_INVOKABLE void checkGateStatus(LoginInformation *info);

    void bootCheck(const LoginInformation &info);

private:
    Patcher *patcher = nullptr;

    LauncherCore &window;
    SquareLauncher &launcher;
};