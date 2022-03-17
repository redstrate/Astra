#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QFuture>
#include <QSettings>
#include <QUuid>
#include <QProcess>
#include <QMessageBox>

class SapphireLauncher;
class SquareLauncher;
class SquareBoot;
class AssetUpdater;
class Watchdog;

enum class GameLicense {
    WindowsStandalone,
    WindowsSteam,
    macOS,
    FreeTrial
};

struct ProfileSettings {
    QUuid uuid;
    QString name;

    // game
    int language = 1; // 1 is english, thats all i know
    QString gamePath, winePath, winePrefixPath;
    QString bootVersion, gameVersion;
    int installedMaxExpansion = -1;
    QList<QString> expansionVersions;
    bool enableWatchdog = false;

    // wine
    // 0 = system, 1 = custom, 2 = built-in (mac only)
    // TODO: yes, i know this should be an enum
#if defined(Q_OS_MAC)
    int wineVersion = 2;
#else
    int wineVersion = 0;
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
    } dalamud;

    // login
    bool encryptArguments = true;
    bool isSapphire = false;
    QString lobbyURL;
    bool rememberUsername = false, rememberPassword = false;

    GameLicense license = GameLicense::WindowsStandalone;
};

struct AppSettings {
    bool closeWhenLaunched = true;
};

struct LoginInformation {
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
public:
    LauncherCore();
    ~LauncherCore();

    QNetworkAccessManager* mgr;

    ProfileSettings getProfile(int index) const;
    ProfileSettings& getProfile(int index);

    int getProfileIndex(QString name);
    QList<QString> profileList() const;
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
    void launchExecutable(const ProfileSettings& settings, QProcess* process, QStringList args);

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

    int defaultProfileIndex = 0;
signals:
    void settingsChanged();
    void successfulLaunch();
    void gameClosed();

private:
    void readExpansionVersions(ProfileSettings& info, int max);
    bool checkIfInPath(QString program);

    QString getDefaultGamePath();
    QString getDefaultWinePrefixPath();
    int getDefaultWineVersion();

    QVector<ProfileSettings> profileSettings;
};
