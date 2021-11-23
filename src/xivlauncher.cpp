#include <QPushButton>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDir>
#include <QFormLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QComboBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCheckBox>
#include <keychain.h>
#include <QMessageBox>
#include <QMenuBar>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QRegularExpressionMatch>

#if defined(Q_OS_MAC)
#include <sys/sysctl.h>
#include <mach/mach_time.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "xivlauncher.h"
#include "sapphirelauncher.h"
#include "squarelauncher.h"
#include "squareboot.h"
#include "settingswindow.h"
#include "blowfish.h"
#include "assetupdater.h"

void LauncherWindow::setSSL(QNetworkRequest& request) {
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherWindow::buildRequest(QNetworkRequest& request) {
    setSSL(request);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QString("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString(QSysInfo::bootUniqueId())));
    request.setRawHeader("Accept",
                         "image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, application/x-ms-xbap, */*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept-Language", "en-us");
}

// from xivdev
char ChecksumTable[] = {
        'f', 'X', '1', 'p', 'G', 't', 'd', 'S',
        '5', 'C', 'A', 'P', '4', '_', 'V', 'L'
};

char GetChecksum(unsigned int key) {
    auto value = key & 0x000F0000;
    return ChecksumTable[value >> 16];
}

#if defined(Q_OS_MAC)
// this is pretty much what wine does :-0
uint32_t TickCount() {
    struct mach_timebase_info convfact;
    mach_timebase_info(&convfact);

    return (mach_absolute_time() * convfact.numer) / (convfact.denom * 1000000);
}
#endif

#if defined(Q_OS_LINUX)
uint32_t TickCount() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if defined(Q_OS_WIN)
uint32_t TickCount() {
    return GetTickCount();
}
#endif

QString encryptGameArg(QString arg) {
    unsigned int rawTicks = TickCount();
    unsigned int ticks = rawTicks & 0xFFFFFFFFu;
    unsigned int key = ticks & 0xFFFF0000u;

    char buffer[9] = {};
    sprintf(buffer, "%08x", key);

    Blowfish session(QByteArray(buffer, 8));
    QByteArray encryptedArg = session.Encrypt((QString(" /T =%1").arg(ticks) + arg).toUtf8());
    QString base64 = encryptedArg.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals);
    char checksum = GetChecksum(key);

    return QString("//**sqex0003%1%2**//").arg(base64, QString(checksum));
}

void LauncherWindow::launchGame(const LoginAuth auth) {
    QList<QString> arguments;

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if(currentProfile().enableDalamud) {
        arguments.push_back(dataDir + "/NativeLauncher.exe");
    }

    // now for the actual game...
    if(currentProfile().useDX9) {
        arguments.push_back(currentProfile().gamePath + "\\game\\ffxiv.exe");
    } else {
        arguments.push_back(currentProfile().gamePath + "\\game\\ffxiv_dx11.exe");
    }

    struct Argument {
        QString key, value;
    };

    QList<Argument> gameArgs;
    gameArgs.push_back({"DEV.DataPathType", QString::number(1)});
    gameArgs.push_back({"DEV.UseSqPack", QString::number(1)});

    gameArgs.push_back({"DEV.MaxEntitledExpansionID", QString::number(auth.maxExpansion)});
    gameArgs.push_back({"DEV.TestSID", auth.SID});
    gameArgs.push_back({"SYS.Region", QString::number(auth.region)});
    gameArgs.push_back({"language", QString::number(currentProfile().language)});
    gameArgs.push_back({"ver", currentProfile().gameVersion});

    if(!auth.lobbyhost.isEmpty()) {
        gameArgs.push_back({"DEV.GMServerHost", auth.frontierHost});
        for(int i = 1; i < 9; i++) {
            gameArgs.push_back({QString("DEV.LobbyHost0%1").arg(QString::number(i)), auth.lobbyhost});
            gameArgs.push_back({QString("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
        }
    }

    auto gameProcess = new QProcess(this);

    if(currentProfile().enableDalamud) {
        connect(gameProcess, &QProcess::readyReadStandardOutput, [this, gameProcess] {
            QString output = gameProcess->readAllStandardOutput();

            auto dalamudProcess = new QProcess();

            QStringList dalamudEnv = gameProcess->environment();

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
            dalamudEnv << "XL_WINEONLINUX=true";
#endif

            dalamudProcess->setEnvironment(dalamudEnv);

            QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

            dalamudProcess->start(currentProfile().winePath, {dataDir + "/Dalamud/" + "Dalamud.Injector.exe", output});
        });
    }

    if(currentProfile().encryptArguments) {
        QString argJoined;
        for(auto arg : gameArgs) {
            argJoined += QString(" /%1 =%2").arg(arg.key, arg.value);
        }

        auto earg = encryptGameArg(argJoined);
        arguments.append(earg);
         launchExecutable(gameProcess, arguments);
    } else {
        for(auto arg : gameArgs) {
            arguments.push_back(QString(" %1=%2").arg(arg.key, arg.value));
        }

        launchExecutable(gameProcess, arguments);
    }
}

void LauncherWindow::launchExecutable(const QStringList args) {
    auto process = new QProcess(this);
    launchExecutable(process, args);
}

void LauncherWindow::launchExecutable(QProcess* process, const QStringList args) {
    QList<QString> arguments;
    QStringList env = QProcess::systemEnvironment();

#if defined(Q_OS_LINUX)
    if(currentProfile().useGamescope) {
        arguments.push_back("gamescope");
        arguments.push_back("-f");
        arguments.push_back("-b");
    }

    if(currentProfile().useGamemode)
        arguments.push_back("gamemoderun");

    if(currentProfile().useEsync) {
        env << "WINEESYNC=1";
    }
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env << "WINEPREFIX=" + currentProfile().winePrefixPath;

    if(currentProfile().enableDXVKhud)
        env << "DXVK_HUD=full";

    arguments.push_back(currentProfile().winePath);
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    process->setWorkingDirectory(currentProfile().gamePath + "/game/");
    process->setEnvironment(env);

    process->start(executable, arguments);
}

QString LauncherWindow::readVersion(QString path) {
    QFile file(path);
    file.open(QFile::OpenModeFlag::ReadOnly);

    return file.readAll();
}

void LauncherWindow::readInitialInformation() {
    defaultProfileIndex = settings.value("defaultProfile", 0).toInt();

    auto profiles = settings.childGroups();

    // create the Default profile if it doesnt exist
    if(profiles.empty())
        profiles.append(QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces));

    profileSettings.resize(profiles.size());

    for(const auto& uuid : profiles) {
        ProfileSettings profile;
        profile.uuid = QUuid(uuid);

        settings.beginGroup(uuid);

        profile.name = settings.value("name", "Default").toString();

#if defined(Q_OS_MAC)
        profile.wineVersion = settings.value("wineVersion", 2).toInt();
#else
        profile.wineVersion = settings.value("wineVersion", 0).toInt();
#endif
        readWineInfo(profile);

        if(settings.contains("gamePath") && settings.value("gamePath").canConvert<QString>() && !settings.value("gamePath").toString().isEmpty()) {
            profile.gamePath = settings.value("gamePath").toString();
        } else {
#if defined(Q_OS_WIN)
            profile.gamePath = "C:\\Program Files (x86)\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_MAC)
            profile.gamePath = QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_LINUX)
            profile.gamePath = QDir::homePath() + "/.wine/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif
        }

        if(settings.contains("winePrefixPath") && settings.value("winePrefixPath").canConvert<QString>() && !settings.value("winePrefixPath").toString().isEmpty()) {
            profile.winePrefixPath = settings.value("winePrefixPath").toString();
        } else {
#if defined(Q_OS_MACOS)
            profile.winePrefixPath = QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy";
#endif

#if defined(Q_OS_LINUX)
            profile.winePrefixPath = QDir::homePath() + "/.wine";
#endif
        }

        // login
        profile.encryptArguments = settings.value("encryptArguments", false).toBool();
        profile.isSapphire = settings.value("isSapphire", false).toBool();
        profile.lobbyURL = settings.value("lobbyURL", "").toString();
        profile.rememberUsername = settings.value("rememberUsername", false).toBool();
        profile.rememberPassword = settings.value("rememberPassword", false).toBool();

        profile.useDX9 = settings.value("useDX9", false).toBool();
        profile.useEsync = settings.value("useEsync", false).toBool();
        profile.useGamemode = settings.value("useGamemode", false).toBool();
        profile.useGamescope = settings.value("useGamescope", false).toBool();
        profile.enableDXVKhud = settings.value("enableDXVKhud", false).toBool();

        profile.enableDalamud = settings.value("enableDalamud", false).toBool();

        profileSettings[settings.value("index").toInt()] = profile;

        settings.endGroup();
    }

    readGameVersion();
}

void LauncherWindow::readWineInfo(ProfileSettings& profile) {
#if defined(Q_OS_MAC)
    switch(profile.wineVersion) {
        case 0: // system wine
            profile.winePath = "/usr/local/bin/wine64";
            break;
        case 1: // custom path
            profile.winePath = profile.winePath;
            break;
        case 2: // ffxiv built-in (for mac users)
            profile.winePath = "/Applications/FINAL FANTASY XIV ONLINE.app/Contents/SharedSupport/finalfantasyxiv/FINAL FANTASY XIV ONLINE/wine";
            break;
    }
#endif

#if defined(Q_OS_LINUX)
    switch(profile.wineVersion) {
            case 0: // system wine (should be in $PATH)
                profile.winePath = "wine";
                break;
            case 1: // custom pth
                profile.winePath = profile.winePath;
                break;
        }
#endif
}

void LauncherWindow::readGameVersion() {
    for(auto& profile : profileSettings) {
        profile.bootVersion = readVersion(profile.gamePath + "/boot/ffxivboot.ver");
        profile.gameVersion = readVersion(profile.gamePath + "/game/ffxivgame.ver");
    }
}

LauncherWindow::LauncherWindow(QWidget* parent) :
        QMainWindow(parent), settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName()) {
    mgr = new QNetworkAccessManager();
    sapphireLauncher = new SapphireLauncher(*this);
    squareLauncher = new SquareLauncher(*this);
    squareBoot = new SquareBoot(*this, *squareLauncher);
    assetUpdater = new AssetUpdater(*this);

    readInitialInformation();

    QMenu* fileMenu = menuBar()->addMenu("File");
    // sorry linux users, for some reason my global menu does not like qt6 apps right now
#if defined(Q_OS_LINUX)
    menuBar()->setNativeMenuBar(false);
#endif

    QAction* settingsAction = fileMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(*this);
        connect(this, &LauncherWindow::settingsChanged, window, &SettingsWindow::reloadControls);
        window->show();
    });

    QMenu* toolsMenu = menuBar()->addMenu("Tools");

    QAction* launchOfficial = toolsMenu->addAction("Launch Official Client...");
    connect(launchOfficial, &QAction::triggered, [=] {
        launchExecutable({currentProfile().gamePath + "/boot/ffxivboot64.exe"});
    });

    QAction* launchSysInfo = toolsMenu->addAction("Launch System Info...");
    connect(launchSysInfo, &QAction::triggered, [=] {
        launchExecutable({currentProfile().gamePath + "/boot/ffxivsysinfo64.exe"});
    });

    QAction* launchCfgBackup = toolsMenu->addAction("Launch Config Backup...");
    connect(launchCfgBackup, &QAction::triggered, [=] {
        launchExecutable({currentProfile().gamePath + "/boot/ffxivconfig64.exe"});
    });

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QMenu* wineMenu = toolsMenu->addMenu("Wine");

    QAction* wineCfg = wineMenu->addAction("winecfg");
    connect(wineCfg, &QAction::triggered, [=] {
        launchExecutable({"winecfg.exe"});
    });

    QAction* controlPanel = wineMenu->addAction("Control Panel");
    connect(controlPanel, &QAction::triggered, [=] {
        launchExecutable({"control.exe"});
    });
#endif

    auto layout = new QFormLayout();

    profileSelect = new QComboBox();
    connect(profileSelect, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        reloadControls();
    });

    layout->addRow("Profile", profileSelect);

    usernameEdit = new QLineEdit();
    layout->addRow("Username", usernameEdit);

    rememberUsernameBox = new QCheckBox();
    connect(rememberUsernameBox, &QCheckBox::stateChanged, [=](int) {
        currentProfile().rememberUsername = rememberUsernameBox->isChecked();
        saveSettings();
    });
    layout->addRow("Remember Username?", rememberUsernameBox);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);
    layout->addRow("Password", passwordEdit);

    rememberPasswordBox = new QCheckBox();
    connect(rememberPasswordBox, &QCheckBox::stateChanged, [=](int) {
        currentProfile().rememberPassword = rememberPasswordBox->isChecked();
        saveSettings();
    });
    layout->addRow("Remember Password?", rememberPasswordBox);

    otpEdit = new QLineEdit();
    layout->addRow("One-Time Password", otpEdit);

    auto loginButton = new QPushButton("Login");
    layout->addRow(loginButton);

    registerButton = new QPushButton("Register");
    layout->addRow(registerButton);

    auto emptyWidget = new QWidget();
    emptyWidget->setLayout(layout);
    setCentralWidget(emptyWidget);

    connect(assetUpdater, &AssetUpdater::finishedUpdating, [=] {
        auto info = LoginInformation{usernameEdit->text(), passwordEdit->text(), otpEdit->text()};

        if(currentProfile().rememberUsername) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(usernameEdit->text());
            job->setKey(currentProfile().name + "-username");
            job->start();
        }

        if(currentProfile().rememberPassword) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(passwordEdit->text());
            job->setKey(currentProfile().name + "-password");
            job->start();
        }

        if(currentProfile().isSapphire) {
            sapphireLauncher->login(currentProfile().lobbyURL, info);
        } else {
            squareBoot->bootCheck(info);
        }
    });

    connect(loginButton, &QPushButton::released, [=] {
        // update the assets first if needed, then it calls the slot above :-)
        assetUpdater->update();
    });

    connect(registerButton, &QPushButton::released, [=] {
        if(currentProfile().isSapphire) {
            auto info = LoginInformation{usernameEdit->text(), passwordEdit->text(), otpEdit->text()};
            sapphireLauncher->registerAccount(currentProfile().lobbyURL, info);
        }
    });

    reloadControls();
}

LauncherWindow::~LauncherWindow() = default;

ProfileSettings LauncherWindow::currentProfile() const {
    return getProfile(profileSelect->currentIndex());
}

ProfileSettings& LauncherWindow::currentProfile() {
    return getProfile(profileSelect->currentIndex());
}

ProfileSettings LauncherWindow::getProfile(int index) const {
    return profileSettings[index];
}

ProfileSettings& LauncherWindow::getProfile(int index) {
    return profileSettings[index];
}

int LauncherWindow::getProfileIndex(QString name) {
    for(int i = 0; i < profileSettings.size(); i++) {
        if(profileSettings[i].name == name)
            return i;
    }

    return -1;
}

QList<QString> LauncherWindow::profileList() const {
    QList<QString> list;
    for(auto profile : profileSettings) {
        list.append(profile.name);
    }

    return list;
}

int LauncherWindow::addProfile() {
    ProfileSettings newProfile;
    newProfile.uuid = QUuid::createUuid();
    newProfile.name = "New Profile";

    profileSettings.append(newProfile);

    settingsChanged();

    return profileSettings.size() - 1;
}

int LauncherWindow::deleteProfile(QString name) {
    int index = 0;
    for(int i = 0; i < profileSettings.size(); i++) {
        if(profileSettings[i].name == name)
            index = i;
    }

    // remove group so it doesnt stay
    settings.beginGroup(profileSettings[index].uuid.toString(QUuid::StringFormat::WithoutBraces));
    settings.remove("");
    settings.endGroup();

    profileSettings.removeAt(index);

    return index - 1;
}

void LauncherWindow::saveSettings() {
    settings.setValue("defaultProfile", defaultProfileIndex);

    for(int i = 0; i < profileSettings.size(); i++) {
        const auto& profile = profileSettings[i];

        settings.beginGroup(profile.uuid.toString(QUuid::StringFormat::WithoutBraces));

        settings.setValue("name", profile.name);
        settings.setValue("index", i);

        // game
        settings.setValue("useDX9", profile.useDX9);
        settings.setValue("gamePath", profile.gamePath);

        // wine
        settings.setValue("wineVersion", profile.wineVersion);
        settings.setValue("winePath", profile.winePath);
        settings.setValue("winePrefixPath", profile.winePrefixPath);

        settings.setValue("useEsync", profile.useEsync);
        settings.setValue("useGamescope", profile.useGamescope);
        settings.setValue("useGamemode", profile.useGamemode);

        // login
        settings.setValue("encryptArguments", profile.encryptArguments);
        settings.setValue("isSapphire", profile.isSapphire);
        settings.setValue("lobbyURL", profile.lobbyURL);
        settings.setValue("rememberUsername", profile.rememberUsername);
        settings.setValue("rememberPassword", profile.rememberPassword);

        settings.setValue("enableDalamud", profile.enableDalamud);

        settings.endGroup();
    }
}

void LauncherWindow::reloadControls() {
    if(currentlyReloadingControls)
        return;

    currentlyReloadingControls = true;

    const int oldIndex = profileSelect->currentIndex();

    profileSelect->clear();

    for(const auto& profile : profileList()) {
        profileSelect->addItem(profile);
    }

    profileSelect->setCurrentIndex(oldIndex);

    if(profileSelect->currentIndex() == -1) {
        profileSelect->setCurrentIndex(defaultProfileIndex);
    }

    rememberUsernameBox->setChecked(currentProfile().rememberUsername);
    if(currentProfile().rememberUsername) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey(currentProfile().name + "-username");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            usernameEdit->setText(job->textData());
        });
    }

    rememberPasswordBox->setChecked(currentProfile().rememberPassword);
    if(currentProfile().rememberPassword) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey(currentProfile().name + "-password");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            passwordEdit->setText(job->textData());
        });
    }

    registerButton->setEnabled(currentProfile().isSapphire);
    otpEdit->setEnabled(!currentProfile().isSapphire);

    currentlyReloadingControls = false;
}