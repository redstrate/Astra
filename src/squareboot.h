#pragma once

#include "launchercore.h"

class SquareLauncher;

class SquareBoot : public QObject {
public:
    SquareBoot(LauncherCore& window, SquareLauncher& launcher);

    void bootCheck(LoginInformation& info);

private:
    LauncherCore& window;
    SquareLauncher& launcher;
};