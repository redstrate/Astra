#pragma once

#include <QProgressDialog>
#include "launchercore.h"

class SquareLauncher;

class SquareBoot : public QObject {
public:
    SquareBoot(LauncherCore& window, SquareLauncher& launcher);

    void bootCheck(LoginInformation& info);

private:
    QProgressDialog* dialog;

    LauncherCore& window;
    SquareLauncher& launcher;
};