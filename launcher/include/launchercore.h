// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFuture>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QtQml>

#include "accountmanager.h"
#include "headline.h"
#include "profile.h"
#include "profilemanager.h"
#include "squareboot.h"
#include "steamapi.h"

class SapphireLauncher;
class SquareLauncher;
class AssetUpdater;
class Watchdog;
class GameInstaller;

class LoginInformation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString password MEMBER password)
    Q_PROPERTY(QString oneTimePassword MEMBER oneTimePassword)
    Q_PROPERTY(Profile *profile MEMBER profile)

public:
    LoginInformation(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    Profile *profile = nullptr;

    QString username, password, oneTimePassword;
};

struct LoginAuth {
    QString SID;
    int region = 2; // america?
    int maxExpansion = 1;

    // if empty, dont set on the client
    QString lobbyhost, frontierHost;
};

class LauncherCore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool loadingFinished READ isLoadingFinished NOTIFY loadingFinished)
    Q_PROPERTY(bool hasAccount READ hasAccount NOTIFY accountChanged)
    Q_PROPERTY(bool isSteam READ isSteam CONSTANT)
    Q_PROPERTY(bool isSteamDeck READ isSteamDeck CONSTANT)
    Q_PROPERTY(SquareBoot *squareBoot MEMBER squareBoot)
    Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)
    Q_PROPERTY(AccountManager *accountManager READ accountManager CONSTANT)
    Q_PROPERTY(bool closeWhenLaunched READ closeWhenLaunched WRITE setCloseWhenLaunched NOTIFY closeWhenLaunchedChanged)
    Q_PROPERTY(bool showNews READ showNews WRITE setShowNews NOTIFY showNewsChanged)
    Q_PROPERTY(Headline *headline READ headline NOTIFY newsChanged)

public:
    explicit LauncherCore(bool isSteam);

    QNetworkAccessManager *mgr;

    ProfileManager *profileManager();
    AccountManager *accountManager();

    /*
     * Begins the login process, and may call SquareBoot or SapphireLauncher depending on the profile type.
     * It's designed to be opaque as possible to the caller.
     *
     * The login process is asynchronous.
     */
    Q_INVOKABLE void login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword);

    /*
     * Attempts to log into a profile without LoginInformation, which may or may not work depending on a combination of
     * the password failing, OTP not being available to auto-generate, among other things.
     *
     * The launcher will still warn the user about any possible errors, however the call site will need to check the
     * result to see whether they need to "reset" or show a failed state or not.
     */
    bool autoLogin(Profile &settings);

    /*
     * Launches the game using the provided authentication.
     */
    void launchGame(const Profile &settings, const LoginAuth &auth);

    /*
     * This just wraps it in wine if needed.
     */
    void launchExecutable(const Profile &settings, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup);

    void addRegistryKey(const Profile &settings, QString key, QString value, QString data);

    void buildRequest(const Profile &settings, QNetworkRequest &request);
    void setSSL(QNetworkRequest &request);
    void readInitialInformation();

    SapphireLauncher *sapphireLauncher = nullptr;
    SquareBoot *squareBoot = nullptr;
    SquareLauncher *squareLauncher = nullptr;
    Watchdog *watchdog = nullptr;

    bool gamescopeAvailable = false;
    bool gamemodeAvailable = false;

    bool closeWhenLaunched() const;
    void setCloseWhenLaunched(bool value);

    bool showNews() const;
    void setShowNews(bool value);

    int defaultProfileIndex = 0;

    bool m_isSteam = false;

    Q_INVOKABLE GameInstaller *createInstaller(Profile *profile);

    bool isLoadingFinished() const;
    bool hasAccount() const;
    bool isSteam() const;
    bool isSteamDeck() const;

    Q_INVOKABLE void refreshNews();
    Headline *headline();

    Q_INVOKABLE void openOfficialLauncher(Profile *profile);
    Q_INVOKABLE void openSystemInfo(Profile *profile);
    Q_INVOKABLE void openConfigBackup(Profile *profile);

signals:
    void loadingFinished();
    void gameInstallationChanged();
    void accountChanged();
    void settingsChanged();
    void successfulLaunch();
    void gameClosed();
    void closeWhenLaunchedChanged();
    void showNewsChanged();
    void loginError(QString message);
    void stageChanged(QString message);
    void stageIndeterminate();
    void stageDeterminate(int min, int max, int value);
    void newsChanged();

private:
    /*
     * Begins the game executable, but calls to Dalamud if needed.
     */
    void beginGameExecutable(const Profile &settings, const LoginAuth &auth);

    /*
     * Starts a vanilla game session with no Dalamud injection.
     */
    void beginVanillaGame(const QString &gameExecutablePath, const Profile &profile, const LoginAuth &auth);

    /*
     * Starts a game session with Dalamud injected.
     */
    void beginDalamudGame(const QString &gameExecutablePath, const Profile &profile, const LoginAuth &auth);

    /*
     * Returns the game arguments needed to properly launch the game. This encrypts it too if needed, and it's already
     * joined!
     */
    QString getGameArgs(const Profile &profile, const LoginAuth &auth);

    bool checkIfInPath(const QString &program);

    SteamAPI *steamApi = nullptr;

    bool m_loadingFinished = false;

    ProfileManager *m_profileManager = nullptr;
    AccountManager *m_accountManager = nullptr;

    Headline *m_headline = nullptr;
};
