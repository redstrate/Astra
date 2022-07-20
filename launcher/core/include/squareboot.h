#pragma once

#include "patcher.h"

class SquareLauncher;
class LauncherCore;
struct LoginInformation;

class SquareBoot : public QObject {
    Q_OBJECT
public:
    SquareBoot(LauncherCore& window, SquareLauncher& launcher);

    Q_INVOKABLE void checkGateStatus(LoginInformation* info);

    void bootCheck(const LoginInformation& info);

private:
    Patcher* patcher = nullptr;

    LauncherCore& window;
    SquareLauncher& launcher;
};