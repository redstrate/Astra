#pragma once

#include "xivlauncher.h"

class SquareLauncher;

class SquareBoot : public QObject {
public:
    SquareBoot(LauncherWindow& window, SquareLauncher& launcher);

    void bootCheck(LoginInformation& info);

private:
    LauncherWindow& window;
    SquareLauncher& launcher;
};