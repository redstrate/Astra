#pragma once

#include "launchercore.h"

class SquareLauncher : public QObject {
    Q_OBJECT
public:
    SquareLauncher(LauncherCore& window);

    void gateOpen();

    void getStored(const LoginInformation& info);

    void login(const LoginInformation& info, QUrl referer);

    void registerSession(const LoginInformation& info);

    bool isGateOpen = false;

signals:
    void gateStatusRecieved(bool gateOpen);

private:
    QString getBootHash(const LoginInformation& info);
    void readExpansionVersions(const LoginInformation& info, int max);

    QString stored, SID;
    LoginAuth auth;

    LauncherCore& window;

    QList<QString> expansionVersions;
};