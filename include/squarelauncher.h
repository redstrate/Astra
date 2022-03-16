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

    QString stored, SID, username;
    LoginAuth auth;

    LauncherCore& window;
};
