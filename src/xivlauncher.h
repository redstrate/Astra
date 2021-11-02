#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QFuture>
#include <QSettings>

class SapphireLauncher;
class SquareLauncher;
class SquareBoot;

struct LoginInformation {
    QString username, password, oneTimePassword;
};

struct LoginAuth {
    QString SID;
    int region = 2; // america?
    int maxExpansion = 1;

    // if empty, dont set on the client
    QString lobbyhost, frontierHost;
};

class LauncherWindow : public QMainWindow {
Q_OBJECT
public:
    explicit LauncherWindow(QWidget* parent = nullptr);

    ~LauncherWindow() override;

    QNetworkAccessManager* mgr;

    int language = 1; // 1 is english, thats all i know
    QString gamePath;
    QString bootVersion, gameVersion;

    bool useEsync, useGamescope, useGamemode;

    void launchGame(const LoginAuth auth);
    void launchExecutable(const QStringList args);
    void buildRequest(QNetworkRequest& request);
    void setSSL(QNetworkRequest& request);
    QString readVersion(QString path);
    void readInitialInformation();

    QSettings settings;

private:
    SapphireLauncher* sapphireLauncher;
    SquareBoot* squareBoot;
    SquareLauncher* squareLauncher;
};
