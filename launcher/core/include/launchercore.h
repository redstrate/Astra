#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QFuture>
#include <QSettings>
#include <QUuid>
#include <QProcess>
#include <QMessageBox>
#include <QtQml>

#include "squareboot.h"

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
    XIVOnMac // macos only
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
    bool useOneTimePassword = false;

    GameLicense license = GameLicense::WindowsStandalone;
    bool isFreeTrial = false;
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

    int getProfileIndex(QString name);
    Q_INVOKABLE QList<QString> profileList() const;
    int addProfile();
    int deleteProfile(QString name);

    void launchGame(const ProfileSettings& settings, LoginAuth auth);

    void launchExecutable(const ProfileSettings& settings, QStringList args);

    /*
     * Used for processes that should be wrapped in gamescope, etc.
     */
    void launchGameExecutable(const ProfileSettings& settings, QProcess* process, QStringList args);

    /*
     * This just wraps it in wine if needed.
     */
    void launchExecutable(const ProfileSettings& settings, QProcess* process, QStringList args, bool isGame, bool needsRegistrySetup);

    /*
     * Launches an external tool. Gamescope for example is intentionally excluded.
     */
    void launchExternalTool(const ProfileSettings& settings, QStringList args);

    void addRegistryKey(const ProfileSettings& settings, QString key, QString value, QString data);

    void buildRequest(const ProfileSettings& settings, QNetworkRequest& request);
    void setSSL(QNetworkRequest& request);
    QString readVersion(QString path);
    void readInitialInformation();
    void readGameVersion();
    void readWineInfo(ProfileSettings& settings);
    void saveSettings();

    void addUpdateButtons(const ProfileSettings& settings, QMessageBox& messageBox);

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
    QString nativeLauncherVersion;

    int defaultProfileIndex = 0;

    QVector<QString> expansionNames;

signals:
    void settingsChanged();
    void successfulLaunch();
    void gameClosed();

private:
    bool checkIfInPath(QString program);
    void readGameData(ProfileSettings& profile);

    QString getDefaultGamePath();
    QString getDefaultWinePrefixPath();

    QVector<ProfileSettings*> profileSettings;
};
