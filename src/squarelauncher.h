#pragma once

#include "launchercore.h"

class SquareLauncher : public QObject {
public:
    SquareLauncher(LauncherCore& window);

    void getStored(const LoginInformation& info);

    void login(const LoginInformation& info, QUrl referer);

    void registerSession(const LoginInformation& info);

private:
    QString getBootHash(const LoginInformation& info);
    void readExpansionVersions(const LoginInformation& info, int max);

    QString stored, SID;
    LoginAuth auth;

    LauncherCore& window;

    QList<QString> expansionVersions;
};