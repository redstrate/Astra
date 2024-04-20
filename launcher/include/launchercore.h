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
#include "steamapi.h"

class SapphireLogin;
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

    // if empty, don't set on the client
    QString lobbyhost, frontierHost;
};

class LauncherCore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool loadingFinished READ isLoadingFinished NOTIFY loadingFinished)
    Q_PROPERTY(bool isSteam READ isSteam CONSTANT)
    Q_PROPERTY(bool isSteamDeck READ isSteamDeck CONSTANT)
    Q_PROPERTY(LauncherSettings *settings READ settings CONSTANT)
    Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)
    Q_PROPERTY(AccountManager *accountManager READ accountManager CONSTANT)
    Q_PROPERTY(Headline *headline READ headline NOTIFY newsChanged)
    Q_PROPERTY(Profile *currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(Profile *autoLoginProfile READ autoLoginProfile WRITE setAutoLoginProfile NOTIFY autoLoginProfileChanged)
    Q_PROPERTY(QString cachedLogoImage READ cachedLogoImage NOTIFY cachedLogoImageChanged)

public:
    LauncherCore();

    /// Initializes the Steamworks API.
    void initializeSteam();

    /// Begins the login process, and may call SquareBoot or SapphireLauncher depending on the profile type.
    /// It's designed to be opaque as possible to the caller.
    /// \note The login process is asynchronous.
    Q_INVOKABLE void login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword);

    /// Attempts to log into a profile without LoginInformation, which may or may not work depending on a combination of the password failing, OTP not being
    /// available to auto-generate, among other things. The launcher will still warn the user about any possible errors, however the call site will need to
    /// check the result to see whether they need to "reset" or show a failed state or not. \note The login process is asynchronous.
    Q_INVOKABLE bool autoLogin(Profile *profile);

    Q_INVOKABLE GameInstaller *createInstaller(Profile *profile);
    Q_INVOKABLE GameInstaller *createInstallerFromExisting(Profile *profile, const QString &filePath);
    Q_INVOKABLE CompatibilityToolInstaller *createCompatInstaller();
    Q_INVOKABLE BenchmarkInstaller *createBenchmarkInstaller(Profile *profile);
    Q_INVOKABLE BenchmarkInstaller *createBenchmarkInstallerFromExisting(Profile *profile, const QString &filePath);

    Q_INVOKABLE void clearAvatarCache();
    Q_INVOKABLE void refreshNews();
    Q_INVOKABLE void refreshLogoImage();

    [[nodiscard]] Profile *currentProfile() const;
    void setCurrentProfile(Profile *profile);

    [[nodiscard]] QString autoLoginProfileName() const;
    [[nodiscard]] Profile *autoLoginProfile() const;
    void setAutoLoginProfile(Profile *value);

    // Networking misc.
    void buildRequest(const Profile &settings, QNetworkRequest &request);
    void setupIgnoreSSL(QNetworkReply *reply);

    [[nodiscard]] bool isLoadingFinished() const;
    [[nodiscard]] bool isSteam() const;
    [[nodiscard]] bool isSteamDeck() const;
    [[nodiscard]] Q_INVOKABLE bool isPatching() const;

    [[nodiscard]] QNetworkAccessManager *mgr();
    [[nodiscard]] LauncherSettings *settings();
    [[nodiscard]] ProfileManager *profileManager();
    [[nodiscard]] AccountManager *accountManager();
    [[nodiscard]] Headline *headline() const;
    [[nodiscard]] QString cachedLogoImage() const;

Q_SIGNALS:
    void loadingFinished();
    void successfulLaunch();
    void gameClosed();
    void loginError(QString message);
    void dalamudError(QString message);
    void miscError(QString message);
    void stageChanged(QString message, QString explanation = {});
    void stageIndeterminate();
    void stageDeterminate(int min, int max, int value);
    void newsChanged();
    void currentProfileChanged();
    void autoLoginProfileChanged();
    void cachedLogoImageChanged();

protected:
    friend class Patcher;

    bool m_isPatching = false;

private:
    QCoro::Task<> beginLogin(LoginInformation &info);

    QCoro::Task<> fetchNews();

    SteamAPI *m_steamApi = nullptr;

    bool m_loadingFinished = false;

    ProfileManager *m_profileManager = nullptr;
    AccountManager *m_accountManager = nullptr;

    SapphireLogin *m_sapphireLogin = nullptr;
    SquareEnixLogin *m_squareEnixLogin = nullptr;

    QNetworkAccessManager *m_mgr = nullptr;
    Headline *m_headline = nullptr;
    LauncherSettings *m_settings = nullptr;
    GameRunner *m_runner = nullptr;
    QString m_cachedLogoImage;

    int m_currentProfileIndex = 0;
};
