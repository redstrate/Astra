#pragma once

#include <QFuture>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QSettings>
#include <QUuid>
#include <QtQml>

#include "squareboot.h"
#include "steamapi.h"

class SapphireLauncher;
class SquareLauncher;
class AssetUpdater;
class Watchdog;

enum class GameLicense {
    WindowsStandalone,
    WindowsSteam,
    macOS
};

enum class WineType {
    System,
    Custom,
    Builtin, // macos only
    XIVOnMac, // macos only
    Proton // steam proton, only available when launched via Steam
};

enum class DalamudChannel {
    Stable,
    Staging,
    Net5
};

class ProfileSettings : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    QUuid uuid;
    QString name;

    // game
    int language = 1; // 1 is english, thats all i know
    QString gamePath, winePath, winePrefixPath;
    QString wineVersion;
    bool enableWatchdog = false;

    BootData* bootData;
    GameData* gameData;

    physis_Repositories repositories;
    const char* bootVersion;

    bool isGameInstalled() const {
        return repositories.repositories_count > 0;
    }

    bool isWineInstalled() const {
        return !wineVersion.isEmpty();
    }

#if defined(Q_OS_MAC)
    WineType wineType = WineType::Builtin;
#else
    WineType wineType = WineType::System;
#endif

    bool useEsync = false, useGamescope = false, useGamemode = false;
    bool useDX9 = false;
    bool enableDXVKhud = false;

    struct GamescopeOptions {
        bool fullscreen = true;
        bool borderless = true;
        int width = 0;
        int height = 0;
        int refreshRate = 0;
    } gamescope;

    struct DalamudOptions {
        bool enabled = false;
        bool optOutOfMbCollection = false;
        DalamudChannel channel = DalamudChannel::Stable;
    } dalamud;

    // login
    bool encryptArguments = true;
    bool isSapphire = false;
    QString lobbyURL;
    bool rememberUsername = false, rememberPassword = false;
    bool rememberOTPSecret = false;
    bool useOneTimePassword = false;
    bool autoLogin = false;

    GameLicense license = GameLicense::WindowsStandalone;
    bool isFreeTrial = false;

    /*
     * Sets a value in the keychain. This function is asynchronous.
     */
    void setKeychainValue(QString key, QString value);

    /*
     * Retrieves a value from the keychain. This function is synchronous.
     */
    QString getKeychainValue(QString key);
};

struct AppSettings {
    bool closeWhenLaunched = true;
    bool showBanners = true;
    bool showNewsList = true;
};

class LoginInformation : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString password MEMBER password)
    Q_PROPERTY(QString oneTimePassword MEMBER oneTimePassword)
    Q_PROPERTY(ProfileSettings* settings MEMBER settings)
    QML_ELEMENT
public:
    ProfileSettings* settings = nullptr;

    QString username, password, oneTimePassword;
};

struct LoginAuth {
    QString SID;
    int region = 2; // america?
    int maxExpansion = 1;

    // if empty, dont set on the client
    QString lobbyhost, frontierHost;
};

class LauncherCore : public QObject {
    Q_OBJECT
    Q_PROPERTY(SquareBoot* squareBoot MEMBER squareBoot)
public:
    LauncherCore();

    // used for qml only, TODO: move this to a dedicated factory
    Q_INVOKABLE LoginInformation* createNewLoginInfo() {
        return new LoginInformation();
    }

    QNetworkAccessManager* mgr;

    ProfileSettings& getProfile(int index);

    // used for qml only
    Q_INVOKABLE ProfileSettings* getProfileQML(int index) {
        return profileSettings[index];
    }

    int getProfileIndex(const QString& name);
    Q_INVOKABLE QList<QString> profileList() const;
    int addProfile();
    int deleteProfile(const QString& name);

    /*
     * Begins the login process, and may call SquareBoot or SapphireLauncher depending on the profile type.
     * It's designed to be opaque as possible to the caller.
     *
     * The login process is asynchronous.
     */
    Q_INVOKABLE void login(LoginInformation* loginInformation);

    /*
     * Attempts to log into a profile without LoginInformation, which may or may not work depending on a combination of
     * the password failing, OTP not being available to auto-generate, among other things.
     *
     * The launcher will still warn the user about any possible errors, however the call site will need to check the
     * result to see whether they need to "reset" or show a failed state or not.
     */
    bool autoLogin(ProfileSettings& settings);

    /*
     * Launches the game using the provided authentication.
     */
    void launchGame(const ProfileSettings& settings, const LoginAuth& auth);

    /*
     * This just wraps it in wine if needed.
     */
    void launchExecutable(
        const ProfileSettings& settings,
        QProcess* process,
        const QStringList& args,
        bool isGame,
        bool needsRegistrySetup);

    void addRegistryKey(const ProfileSettings& settings, QString key, QString value, QString data);

    void buildRequest(const ProfileSettings& settings, QNetworkRequest& request);
    void setSSL(QNetworkRequest& request);
    void readInitialInformation();
    void readGameVersion();
    void readWineInfo(ProfileSettings& settings);
    void saveSettings();

    QSettings settings;

    SapphireLauncher* sapphireLauncher;
    SquareBoot* squareBoot;
    SquareLauncher* squareLauncher;
    AssetUpdater* assetUpdater;
    Watchdog* watchdog;

    bool gamescopeAvailable = false;
    bool gamemodeAvailable = false;

    AppSettings appSettings;

    QString dalamudVersion;
    int dalamudAssetVersion = -1;
    QString runtimeVersion;

    int defaultProfileIndex = 0;

    QVector<QString> expansionNames;

    bool isSteam = false;

signals:
    void settingsChanged();
    void successfulLaunch();
    void gameClosed();

private:
    /*
     * Begins the game executable, but calls to Dalamud if needed.
     */
    void beginGameExecutable(const ProfileSettings& settings, const LoginAuth& auth);

    /*
     * Starts a vanilla game session with no Dalamud injection.
     */
    void beginVanillaGame(const QString& gameExecutablePath, const ProfileSettings& profile, const LoginAuth& auth);

    /*
     * Starts a game session with Dalamud injected.
     */
    void beginDalamudGame(const QString& gameExecutablePath, const ProfileSettings& profile, const LoginAuth& auth);

    /*
     * Returns the game arguments needed to properly launch the game. This encrypts it too if needed, and it's already
     * joined!
     */
    QString getGameArgs(const ProfileSettings& profile, const LoginAuth& auth);

    bool checkIfInPath(const QString& program);
    void readGameData(ProfileSettings& profile);

    QString getDefaultGamePath();
    QString getDefaultWinePrefixPath();

    QVector<ProfileSettings*> profileSettings;

    SteamAPI* steamApi = nullptr;
};
