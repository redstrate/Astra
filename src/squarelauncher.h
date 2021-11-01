#pragma once

#include "xivlauncher.h"

class SquareLauncher : public QObject {
public:
    SquareLauncher(LauncherWindow& window);

    void getStored(const LoginInformation& info);

    void login(const LoginInformation& info, const QUrl referer);

    void registerSession(const LoginInformation& info);

private:
    QString getBootHash();
    void readExpansionVersions(int max);

    QString stored, SID;
    LoginAuth auth;

    LauncherWindow& window;

    QList<QString> expansionVersions;
};