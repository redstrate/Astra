#pragma once

#include <QString>

#include "launchercore.h"

class SapphireLauncher : QObject {
public:
    explicit SapphireLauncher(LauncherCore& window);

    void login(const QString& lobbyUrl, const LoginInformation& info);
    void registerAccount(const QString& lobbyUrl, const LoginInformation& info);

private:
    LauncherCore& window;
};