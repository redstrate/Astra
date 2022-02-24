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

#include "launchercore.h"
#include "sapphirelauncher.h"
#include "squarelauncher.h"
#include "squareboot.h"
#include "settingswindow.h"
#include "blowfish.h"
#include "assetupdater.h"

#ifdef ENABLE_WATCHDOG
#include "watchdog.h"
#endif

void LauncherCore::setSSL(QNetworkRequest& request) {
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherCore::buildRequest(QNetworkRequest& request) {
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

void LauncherCore::launchGame(const ProfileSettings& profile, const LoginAuth auth) {
    QList<QString> arguments;

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if(profile.enableDalamud) {
        arguments.push_back(dataDir + "/NativeLauncher.exe");
    }

    // now for the actual game...
    if(profile.useDX9) {
        arguments.push_back(profile.gamePath + "\\game\\ffxiv.exe");
    } else {
        arguments.push_back(profile.gamePath + "\\game\\ffxiv_dx11.exe");
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
    gameArgs.push_back({"language", QString::number(profile.language)});
    gameArgs.push_back({"ver", profile.gameVersion});

    if(!auth.lobbyhost.isEmpty()) {
        gameArgs.push_back({"DEV.GMServerHost", auth.frontierHost});
        for(int i = 1; i < 9; i++) {
            gameArgs.push_back({QString("DEV.LobbyHost0%1").arg(QString::number(i)), auth.lobbyhost});
            gameArgs.push_back({QString("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
        }
    }

    auto gameProcess = new QProcess(this);
    if(profile.useSteam) {
        gameArgs.push_back({"IsSteam", "1"});
        gameProcess->environment() << "IS_FFXIV_LAUNCH_FROM_STEAM=1";
    }
    gameProcess->setProcessChannelMode(QProcess::MergedChannels);

    if(profile.enableDalamud) {
        connect(gameProcess, &QProcess::readyReadStandardOutput, [this, gameProcess, profile] {
            QString output = gameProcess->readAllStandardOutput();
            bool success;
            int dec = output.toInt(&success, 10);
            if(!success)
              return;
            
            qDebug() << "got output: " << output;

            qDebug() << "Now launching dalamud...";

            auto dalamudProcess = new QProcess();
            dalamudProcess->setProcessChannelMode(QProcess::MergedChannels);

            QStringList dalamudEnv = gameProcess->environment();

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
            dalamudEnv << "XL_WINEONLINUX=true";
#endif

            dalamudProcess->setEnvironment(dalamudEnv);

            QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

            dalamudProcess->start(profile.winePath, {dataDir + "/Dalamud/" + "Dalamud.Injector.exe", output});
        });
    }

    const QString argFormat = profile.encryptArguments ? " /%1 =%2" : "%1=%2";

    QString argJoined;
    for(const auto& arg : gameArgs) {
        argJoined += argFormat.arg(arg.key, arg.value);
    }

    if(profile.encryptArguments) {
        arguments.append(encryptGameArg(argJoined));
    } else {
        arguments.append(argJoined);
    }

    launchExecutable(profile, gameProcess, arguments);
}

void LauncherCore::launchExecutable(const ProfileSettings& profile, const QStringList args) {
    auto process = new QProcess(this);
    launchExecutable(profile, process, args);
}

void LauncherCore::launchExecutable(const ProfileSettings& profile, QProcess* process, const QStringList args) {
    QList<QString> arguments;
    QStringList env = QProcess::systemEnvironment();

#if defined(Q_OS_LINUX)
    if(profile.useGamescope) {
        arguments.push_back("gamescope");

        if(profile.gamescope.fullscreen)
            arguments.push_back("-f");

        if(profile.gamescope.borderless)
            arguments.push_back("-b");

        if(profile.gamescope.width >= 0)
            arguments.push_back("-w " + QString::number(profile.gamescope.width));

        if(profile.gamescope.height >= 0)
            arguments.push_back("-h " + QString::number(profile.gamescope.height));

        if(profile.gamescope.refreshRate >= 0)
            arguments.push_back("-r " + QString::number(profile.gamescope.refreshRate));
    }

    if(profile.useGamemode)
        arguments.push_back("gamemoderun");

    if(profile.useEsync)
        env << "WINEESYNC=1";
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env << "WINEPREFIX=" + profile.winePrefixPath;

    if(profile.enableDXVKhud)
        env << "DXVK_HUD=full";

    arguments.push_back(profile.winePath);
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    process->setWorkingDirectory(profile.gamePath + "/game/");
    process->setEnvironment(env);

    process->start(executable, arguments);
}

QString LauncherCore::readVersion(QString path) {
    QFile file(path);
    file.open(QFile::OpenModeFlag::ReadOnly);

    return file.readAll();
}

void LauncherCore::readInitialInformation() {
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
        profile.encryptArguments = settings.value("encryptArguments", true).toBool();
        profile.isSapphire = settings.value("isSapphire", false).toBool();
        profile.lobbyURL = settings.value("lobbyURL", "").toString();
        profile.rememberUsername = settings.value("rememberUsername", false).toBool();
        profile.rememberPassword = settings.value("rememberPassword", false).toBool();
        profile.useSteam = settings.value("useSteam", false).toBool();

        profile.useDX9 = settings.value("useDX9", false).toBool();
        profile.useEsync = settings.value("useEsync", false).toBool();
        profile.useGamemode = settings.value("useGamemode", false).toBool();
        profile.useGamescope = settings.value("useGamescope", false).toBool();
        profile.enableDXVKhud = settings.value("enableDXVKhud", false).toBool();
        profile.enableWatchdog = settings.value("enableWatchdog", false).toBool();

        // gamescope
        profile.gamescope.fullscreen = settings.value("gamescopeFullscreen", true).toBool();
        profile.gamescope.borderless = settings.value("gamescopeBorderless", true).toBool();
        profile.gamescope.width = settings.value("gamescopeWidth", 0).toInt();
        profile.gamescope.height = settings.value("gamescopeHeight", 0).toInt();
        profile.gamescope.refreshRate = settings.value("gamescopeRefreshRate", 0).toInt();

        profile.enableDalamud = settings.value("enableDalamud", false).toBool();

        profileSettings[settings.value("index").toInt()] = profile;

        settings.endGroup();
    }

    readGameVersion();
}

void LauncherCore::readWineInfo(ProfileSettings& profile) {
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

void LauncherCore::readGameVersion() {
    for(auto& profile : profileSettings) {
        profile.bootVersion = readVersion(profile.gamePath + "/boot/ffxivboot.ver");
        profile.gameVersion = readVersion(profile.gamePath + "/game/ffxivgame.ver");

        for(auto dir : QDir(profile.gamePath + "/game/sqpack/").entryList(QDir::Filter::Dirs)) {
            if(dir.contains("ex") && dir.length() == 3 && dir[2].isDigit()) {
                const int expacVersion = dir[2].digitValue();

                profile.installedMaxExpansion = std::max(profile.installedMaxExpansion, expacVersion);
            }

            if(dir == "ffxiv") {
                profile.installedMaxExpansion = std::max(profile.installedMaxExpansion, 0);
            }
        }

        readExpansionVersions(profile, profile.installedMaxExpansion);
    }
}

LauncherCore::LauncherCore() : settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName()) {
    mgr = new QNetworkAccessManager();
    sapphireLauncher = new SapphireLauncher(*this);
    squareLauncher = new SquareLauncher(*this);
    squareBoot = new SquareBoot(*this, *squareLauncher);
    assetUpdater = new AssetUpdater(*this);

#ifdef ENABLE_WATCHDOG
    watchdog = new Watchdog(*this);
#endif

    readInitialInformation();

    // check gate status before login
    squareLauncher->gateOpen();

    // TODO: we really should call this "heavy" signal
    connect(squareLauncher, &SquareLauncher::gateStatusRecieved, this, &LauncherCore::settingsChanged);
}

LauncherCore::~LauncherCore() noexcept {
#ifdef ENABLE_WATCHDOG
    delete watchdog;
#endif
}

ProfileSettings LauncherCore::getProfile(int index) const {
    return profileSettings[index];
}

ProfileSettings& LauncherCore::getProfile(int index) {
    return profileSettings[index];
}

int LauncherCore::getProfileIndex(QString name) {
    for(int i = 0; i < profileSettings.size(); i++) {
        if(profileSettings[i].name == name)
            return i;
    }

    return -1;
}

QList<QString> LauncherCore::profileList() const {
    QList<QString> list;
    for(auto profile : profileSettings) {
        list.append(profile.name);
    }

    return list;
}

int LauncherCore::addProfile() {
    ProfileSettings newProfile;
    newProfile.uuid = QUuid::createUuid();
    newProfile.name = "New Profile";

    profileSettings.append(newProfile);

    settingsChanged();

    return profileSettings.size() - 1;
}

int LauncherCore::deleteProfile(QString name) {
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

void LauncherCore::saveSettings() {
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

        // gamescope
        settings.setValue("gamescopeFullscreen", profile.gamescope.fullscreen);
        settings.setValue("gamescopeBorderless", profile.gamescope.borderless);
        settings.setValue("gamescopeWidth", profile.gamescope.width);
        settings.setValue("gamescopeHeight", profile.gamescope.height);
        settings.setValue("gamescopeRefreshRate", profile.gamescope.refreshRate);

        // login
        settings.setValue("encryptArguments", profile.encryptArguments);
        settings.setValue("isSapphire", profile.isSapphire);
        settings.setValue("lobbyURL", profile.lobbyURL);
        settings.setValue("rememberUsername", profile.rememberUsername);
        settings.setValue("rememberPassword", profile.rememberPassword);
        settings.setValue("useSteam", profile.useSteam);

        settings.setValue("enableDalamud", profile.enableDalamud);
        settings.setValue("enableWatchdog", profile.enableWatchdog);

        settings.endGroup();
    }
}

void LauncherCore::addUpdateButtons(const ProfileSettings& settings, QMessageBox& messageBox) {
    auto launcherButton = messageBox.addButton("Launch Official Launcher", QMessageBox::NoRole);
    connect(launcherButton, &QPushButton::clicked, [=] {
        launchExecutable(settings, {settings.gamePath + "/boot/ffxivboot.exe"});
    });

    messageBox.addButton(QMessageBox::StandardButton::Ok);
}

void LauncherCore::readExpansionVersions(ProfileSettings& info, int max) {
    info.expansionVersions.clear();

    for(int i = 0; i < max; i++)
        info.expansionVersions.push_back(readVersion(QString("%1/game/sqpack/ex%2/ex%2.ver").arg(info.gamePath, QString::number(i + 1))));
}
