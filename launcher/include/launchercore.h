// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFuture>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QtQml>
#include <qcorotask.h>

#include "accountmanager.h"
#include "headline.h"
#include "launchersettings.h"
#include "profile.h"
#include "profilemanager.h"
#include "squareboot.h"
#include "steamapi.h"

class SapphireLauncher;
class SquareLauncher;
class AssetUpdater;
class GameInstaller;
class CompatibilityToolInstaller;

class LoginInformation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString password MEMBER password)
    Q_PROPERTY(QString oneTimePassword MEMBER oneTimePassword)
    Q_PROPERTY(Profile *profile MEMBER profile)

public:
    explicit LoginInformation(QObject *parent = nullptr)
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

    // if empty, don't set on the client
    QString lobbyhost, frontierHost;
};

class LauncherCore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool loadingFinished READ isLoadingFinished NOTIFY loadingFinished)
    Q_PROPERTY(bool hasAccount READ hasAccount NOTIFY accountChanged)
    Q_PROPERTY(bool isSteam READ isSteam CONSTANT)
    Q_PROPERTY(bool isSteamDeck READ isSteamDeck CONSTANT)
    Q_PROPERTY(LauncherSettings *settings READ settings CONSTANT)
    Q_PROPERTY(SquareBoot *squareBoot MEMBER m_squareBoot)
    Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)
    Q_PROPERTY(AccountManager *accountManager READ accountManager CONSTANT)
    Q_PROPERTY(Headline *headline READ headline NOTIFY newsChanged)
    Q_PROPERTY(Profile *currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(Profile *autoLoginProfile READ autoLoginProfile WRITE setAutoLoginProfile NOTIFY autoLoginProfileChanged)

public:
    LauncherCore();

    QNetworkAccessManager *mgr;

    LauncherSettings *settings();
    ProfileManager *profileManager();
    AccountManager *accountManager();

    void initializeSteam();

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
    Q_INVOKABLE bool autoLogin(Profile *profile);

    /*
     * Launches the game using the provided authentication.
     */
    void launchGame(Profile &settings, const LoginAuth &auth);

    void buildRequest(const Profile &settings, QNetworkRequest &request);
    void setSSL(QNetworkRequest &request);
    void setupIgnoreSSL(QNetworkReply *reply);

    Q_INVOKABLE GameInstaller *createInstaller(Profile *profile);
    Q_INVOKABLE CompatibilityToolInstaller *createCompatInstaller();

    [[nodiscard]] bool isLoadingFinished() const;
    [[nodiscard]] bool hasAccount() const;
    [[nodiscard]] bool isSteam() const;
    [[nodiscard]] bool isSteamDeck() const;

    Q_INVOKABLE void refreshNews();
    [[nodiscard]] Headline *headline() const;

    [[nodiscard]] Profile *currentProfile() const;
    void setCurrentProfile(Profile *profile);

    Q_INVOKABLE void clearAvatarCache();

    [[nodiscard]] QString autoLoginProfileName() const;
    [[nodiscard]] Profile *autoLoginProfile() const;
    void setAutoLoginProfile(Profile *value);

signals:
    void loadingFinished();
    void accountChanged();
    void successfulLaunch();
    void gameClosed();
    void loginError(QString message);
    void dalamudError(QString message);
    void stageChanged(QString message);
    void stageIndeterminate();
    void stageDeterminate(int min, int max, int value);
    void newsChanged();
    void currentProfileChanged();
    void autoLoginProfileChanged();

private:
    /*
     * This just wraps it in wine if needed.
     */
    void launchExecutable(const Profile &settings, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup);

    void addRegistryKey(const Profile &settings, QString key, QString value, QString data);

    void readInitialInformation();

    QCoro::Task<> beginLogin(LoginInformation &info);

    /*
     * Begins the game executable, but calls to Dalamud if needed.
     */
    void beginGameExecutable(Profile &settings, const LoginAuth &auth);

    /*
     * Starts a vanilla game session with no Dalamud injection.
     */
    void beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const LoginAuth &auth);

    /*
     * Starts a game session with Dalamud injected.
     */
    void beginDalamudGame(const QString &gameExecutablePath, Profile &profile, const LoginAuth &auth);

    /*
     * Returns the game arguments needed to properly launch the game. This encrypts it too if needed, and it's already
     * joined!
     */
    QString getGameArgs(const Profile &profile, const LoginAuth &auth);

    bool checkIfInPath(const QString &program);

    QCoro::Task<> fetchNews();

    bool m_isSteam = false;
    SteamAPI *m_steamApi = nullptr;

    bool m_loadingFinished = false;

    ProfileManager *m_profileManager = nullptr;
    AccountManager *m_accountManager = nullptr;

    SapphireLauncher *m_sapphireLauncher = nullptr;
    SquareBoot *m_squareBoot = nullptr;
    SquareLauncher *m_squareLauncher = nullptr;

    Headline *m_headline = nullptr;
    LauncherSettings *m_settings = nullptr;

    int m_currentProfileIndex = 0;
};
