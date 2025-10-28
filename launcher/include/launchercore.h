// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QNetworkAccessManager>
#include <QtQml>
#include <qcorotask.h>

#include "accountmanager.h"
#include "config.h"
#include "headline.h"
#include "profile.h"
#include "profilemanager.h"
#include "steamapi.h"

class SquareEnixLogin;
class AssetUpdater;
class GameInstaller;
class CompatibilityToolInstaller;
class GameRunner;
class BenchmarkInstaller;

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
    Account *account = nullptr;
};

class LauncherCore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool loadingFinished READ isLoadingFinished NOTIFY loadingFinished)
    Q_PROPERTY(bool isSteam READ isSteam CONSTANT)
    Q_PROPERTY(bool isSteamDeck READ isSteamDeck CONSTANT)
    Q_PROPERTY(bool isWindows READ isWindows CONSTANT)
    Q_PROPERTY(Config *config READ config CONSTANT)
    Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)
    Q_PROPERTY(AccountManager *accountManager READ accountManager CONSTANT)
    Q_PROPERTY(Headline *headline READ headline NOTIFY newsChanged)
    Q_PROPERTY(Profile *currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(Profile *autoLoginProfile READ autoLoginProfile WRITE setAutoLoginProfile NOTIFY autoLoginProfileChanged)
    Q_PROPERTY(QString cachedLogoImage READ cachedLogoImage NOTIFY cachedLogoImageChanged)

public:
    LauncherCore();
    ~LauncherCore() override;

    /// Begins the login process.
    /// It's designed to be opaque as possible to the caller.
    /// \note The login process is asynchronous.
    Q_INVOKABLE void login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword);

    /// Attempts to log into a profile without LoginInformation, which may or may not work depending on a combination of the password failing, OTP not being
    /// available to auto-generate, among other things. The launcher will still warn the user about any possible errors, however the call site will need to
    /// check the result to see whether they need to "reset" or show a failed state or not. \note The login process is asynchronous.
    Q_INVOKABLE bool autoLogin(Profile *profile);

    /// Launches the game without patching, or logging in.
    /// Meant to test if we can get to the title screen and is intended to fail to do anything else.
    Q_INVOKABLE void immediatelyLaunch(Profile *profile);

    Q_INVOKABLE GameInstaller *createInstaller(Profile *profile);
    Q_INVOKABLE GameInstaller *createInstallerFromExisting(Profile *profile, const QString &filePath);
    Q_INVOKABLE CompatibilityToolInstaller *createCompatInstaller();
    Q_INVOKABLE BenchmarkInstaller *createBenchmarkInstaller(Profile *profile);
    Q_INVOKABLE BenchmarkInstaller *createBenchmarkInstallerFromExisting(Profile *profile, const QString &filePath);

    /// Fetches the avatar for @p account
    void fetchAvatar(Account *account);
    Q_INVOKABLE void clearAvatarCache();

    Q_INVOKABLE void refreshNews();
    Q_INVOKABLE void refreshLogoImage();

    [[nodiscard]] Profile *currentProfile() const;
    void setCurrentProfile(const Profile *profile);

    [[nodiscard]] QString autoLoginProfileName() const;
    [[nodiscard]] Profile *autoLoginProfile() const;
    void setAutoLoginProfile(const Profile *value);

    // Networking misc.
    void buildRequest(const Profile &settings, QNetworkRequest &request);
    void setupIgnoreSSL(QNetworkReply *reply);

    [[nodiscard]] bool isLoadingFinished() const;
    [[nodiscard]] bool isSteam() const;
    [[nodiscard]] bool isSteamDeck() const;
    [[nodiscard]] static bool isWindows();
    [[nodiscard]] static bool needsCompatibilityTool();
    [[nodiscard]] Q_INVOKABLE bool isPatching() const;

    [[nodiscard]] QNetworkAccessManager *mgr();
    [[nodiscard]] Config *config() const;
    [[nodiscard]] ProfileManager *profileManager();
    [[nodiscard]] AccountManager *accountManager();
    [[nodiscard]] Headline *headline() const;
    [[nodiscard]] QString cachedLogoImage() const;
    [[nodiscard]] SteamAPI *steamApi() const;

    /**
     * @brief Opens the official launcher. Useful if Astra decides not to work that day!
     */
    Q_INVOKABLE void openOfficialLauncher(Profile *profile);

    /**
     * @brief Opens the official system information executable.
     */
    Q_INVOKABLE void openSystemInfo(Profile *profile);

    /**
     * @brief Opens the config backup tool.
     */
    Q_INVOKABLE void openConfigBackup(Profile *profile);

    Q_INVOKABLE void downloadServerConfiguration(Account *account, const QString &url);

Q_SIGNALS:
    void loadingFinished();
    void successfulLaunch();
    void gameClosed(Profile *profile);
    void loginError(QString message);
    void dalamudError(QString message);
    void miscError(QString message);
    void assetError(QString message);
    void stageChanged(QString message, QString explanation = {});
    void stageIndeterminate();
    void stageDeterminate(int min, int max, int value);
    void newsChanged();
    void currentProfileChanged();
    void autoLoginProfileChanged();
    void cachedLogoImageChanged();
    void showWindow();
    void requiresUpdate(QString message);
    void updateDecided(bool allowUpdate);
    void assetDecided(bool shouldContinue);
    void dalamudDecided(bool shouldContinue);

protected:
    friend class Patcher;

    bool m_isPatching = false;

private:
    QCoro::Task<> beginLogin(LoginInformation &info);

    QCoro::Task<> fetchNews();

    void handleGameExit();

    /// Updates FFXIV.cfg with some recommended options like turning the opening cutscene movie off
    void updateConfig(const Account *account);

    /// Tell the system to keep the screen on and don't go to sleep
    void inhibitSleep();

    /// Tell the system we can allow the screen to turn off
    void uninhibitSleep();

    QCoro::Task<> beginAutoConfiguration(Account *account, QString url);

    QString currentProfileId() const;

    SteamAPI *m_steamApi = nullptr;

    bool m_loadingFinished = false;

    ProfileManager *m_profileManager = nullptr;
    AccountManager *m_accountManager = nullptr;

    SquareEnixLogin *m_squareEnixLogin = nullptr;

    QNetworkAccessManager *m_mgr = nullptr;
    Headline *m_headline = nullptr;
    Config *m_config = nullptr;
    GameRunner *m_runner = nullptr;
    QString m_cachedLogoImage;

    int m_currentProfileIndex = 0;

    unsigned int screenSaverDbusCookie = 0;
};
