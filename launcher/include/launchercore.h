// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFuture>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QtQml>
#include <qcorotask.h>

#include "accountmanager.h"
#include "config.h"
#include "headline.h"
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
    Q_PROPERTY(SquareBoot *squareBoot MEMBER m_squareBoot)
    Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)
    Q_PROPERTY(AccountManager *accountManager READ accountManager CONSTANT)
    Q_PROPERTY(bool closeWhenLaunched READ closeWhenLaunched WRITE setCloseWhenLaunched NOTIFY closeWhenLaunchedChanged)
    Q_PROPERTY(bool showNews READ showNews WRITE setShowNews NOTIFY showNewsChanged)
    Q_PROPERTY(bool showDevTools READ showDevTools WRITE setShowDevTools NOTIFY showDevToolsChanged)
    Q_PROPERTY(bool keepPatches READ keepPatches WRITE setKeepPatches NOTIFY keepPatchesChanged)
    Q_PROPERTY(QString dalamudDistribServer READ dalamudDistribServer WRITE setDalamudDistribServer NOTIFY dalamudDistribServerChanged)
    Q_PROPERTY(QString squareEnixServer READ squareEnixServer WRITE setSquareEnixServer NOTIFY squareEnixServerChanged)
    Q_PROPERTY(QString squareEnixLoginServer READ squareEnixLoginServer WRITE setSquareEnixLoginServer NOTIFY squareEnixLoginServerChanged)
    Q_PROPERTY(QString xivApiServer READ xivApiServer WRITE setXivApiServer NOTIFY xivApiServerChanged)
    Q_PROPERTY(QString preferredProtocol READ preferredProtocol WRITE setPreferredProtocol NOTIFY preferredProtocolChanged)
    Q_PROPERTY(bool argumentsEncrypted READ argumentsEncrypted WRITE setArgumentsEncrypted NOTIFY encryptedArgumentsChanged)
    Q_PROPERTY(Headline *headline READ headline NOTIFY newsChanged)
    Q_PROPERTY(Profile *currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(Profile *autoLoginProfile READ autoLoginProfile WRITE setAutoLoginProfile NOTIFY autoLoginProfileChanged)

public:
    LauncherCore();

    QNetworkAccessManager *mgr;

    ProfileManager *profileManager();
    AccountManager *accountManager();

    void setIsSteam(bool isSteam);

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

    /*
     * This just wraps it in wine if needed.
     */
    void launchExecutable(const Profile &settings, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup);

    void addRegistryKey(const Profile &settings, QString key, QString value, QString data);

    void buildRequest(const Profile &settings, QNetworkRequest &request);
    void setSSL(QNetworkRequest &request);
    void setupIgnoreSSL(QNetworkReply *reply);

    void readInitialInformation();

    [[nodiscard]] bool closeWhenLaunched() const;
    void setCloseWhenLaunched(bool value);

    [[nodiscard]] bool showNews() const;
    void setShowNews(bool value);

    [[nodiscard]] bool showDevTools() const;
    void setShowDevTools(bool value);

    [[nodiscard]] bool keepPatches() const;
    void setKeepPatches(bool value);

    [[nodiscard]] QString dalamudDistribServer() const;
    void setDalamudDistribServer(const QString &value);

    [[nodiscard]] QString squareEnixServer() const;
    void setSquareEnixServer(const QString &value);

    [[nodiscard]] QString squareEnixLoginServer() const;
    void setSquareEnixLoginServer(const QString &value);

    [[nodiscard]] QString xivApiServer() const;
    void setXivApiServer(const QString &value);

    [[nodiscard]] QString preferredProtocol() const;
    void setPreferredProtocol(const QString &value);

    [[nodiscard]] bool argumentsEncrypted() const;
    void setArgumentsEncrypted(bool value);

    [[nodiscard]] QString autoLoginProfileName() const;
    [[nodiscard]] Profile *autoLoginProfile() const;
    void setAutoLoginProfile(Profile *value);

    Q_INVOKABLE GameInstaller *createInstaller(Profile *profile);
    Q_INVOKABLE CompatibilityToolInstaller *createCompatInstaller();

    [[nodiscard]] bool isLoadingFinished() const;
    [[nodiscard]] bool hasAccount() const;
    [[nodiscard]] bool isSteam() const;
    [[nodiscard]] bool isSteamDeck() const;

    Q_INVOKABLE void refreshNews();
    [[nodiscard]] Headline *headline() const;

    Q_INVOKABLE void openOfficialLauncher(Profile *profile);
    Q_INVOKABLE void openSystemInfo(Profile *profile);
    Q_INVOKABLE void openConfigBackup(Profile *profile);

    [[nodiscard]] Profile *currentProfile() const;
    void setCurrentProfile(Profile *profile);

    Q_INVOKABLE void clearAvatarCache();

signals:
    void loadingFinished();
    void gameInstallationChanged();
    void accountChanged();
    void settingsChanged();
    void successfulLaunch();
    void gameClosed();
    void closeWhenLaunchedChanged();
    void showNewsChanged();
    void showDevToolsChanged();
    void keepPatchesChanged();
    void dalamudDistribServerChanged();
    void squareEnixServerChanged();
    void squareEnixLoginServerChanged();
    void xivApiServerChanged();
    void preferredProtocolChanged();
    void encryptedArgumentsChanged();
    void loginError(QString message);
    void stageChanged(QString message);
    void stageIndeterminate();
    void stageDeterminate(int min, int max, int value);
    void newsChanged();
    void currentProfileChanged();
    void autoLoginProfileChanged();

private:
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
    Config *m_config = nullptr;

    int m_currentProfileIndex = 0;
};
