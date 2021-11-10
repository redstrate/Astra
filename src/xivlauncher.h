#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QFuture>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QUuid>

class SapphireLauncher;
class SquareLauncher;
class SquareBoot;

struct ProfileSettings {
    QUuid uuid;
    QString name;

    // game
    int language = 1; // 1 is english, thats all i know
    QString gamePath, winePath, winePrefixPath;
    QString bootVersion, gameVersion;

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

    // login
    bool encryptArguments = false;
    bool isSapphire = false;
    QString lobbyURL;
    bool rememberUsername = false, rememberPassword = false;
};

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

    ProfileSettings currentProfile() const;
    ProfileSettings& currentProfile();

    ProfileSettings getProfile(int index) const;
    ProfileSettings& getProfile(int index);

    int getProfileIndex(QString name);
    QList<QString> profileList() const;
    int addProfile();
    int deleteProfile(QString name);

    void launchGame(const LoginAuth auth);
    void launchExecutable(const QStringList args);
    void buildRequest(QNetworkRequest& request);
    void setSSL(QNetworkRequest& request);
    QString readVersion(QString path);
    void readInitialInformation();
    void readGameVersion();
    void readWineInfo(ProfileSettings& settings);
    void saveSettings();

    QSettings settings;

public slots:
    void reloadControls();

signals:
    void settingsChanged();

private:
    bool currentlyReloadingControls = false;

    SapphireLauncher* sapphireLauncher;
    SquareBoot* squareBoot;
    SquareLauncher* squareLauncher;

    QComboBox* profileSelect;
    QLineEdit* usernameEdit, *passwordEdit;
    QLineEdit* otpEdit;
    QCheckBox* rememberUsernameBox, *rememberPasswordBox;
    QPushButton* registerButton;

    QList<ProfileSettings> profileSettings;
    int defaultProfileIndex = 0;
};
