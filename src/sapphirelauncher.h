#pragma once

#include <QString>

#include "launchercore.h"

class SapphireLauncher : QObject {
public:
    SapphireLauncher(LauncherCore& window);

    void login(QString lobbyUrl, const LoginInformation& info);
    void registerAccount(QString lobbyUrl, const LoginInformation& info);

private:
    LauncherCore& window;
};