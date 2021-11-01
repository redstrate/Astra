#pragma once

#include <QString>

#include "xivlauncher.h"

class SapphireLauncher : QObject {
public:
    SapphireLauncher(LauncherWindow& window);

    void login(QString lobbyUrl, const LoginInformation& info);
    void registerAccount(QString lobbyUrl, const LoginInformation& info);

private:
    LauncherWindow& window;
};