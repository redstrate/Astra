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
#include <qt5keychain/keychain.h>
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
#include "encryptedarg.h"

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

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    if(profile.useSteam) {
        gameArgs.push_back({"IsSteam", "1"});
        env.insert("IS_FFXIV_LAUNCH_FROM_STEAM", QString::number(1));
    }
    env.insert("DALAMUD_RUNTIME", "Z:" + dataDir.replace('/', '\\') + "\\DalamudRuntime");

    gameProcess->setProcessEnvironment(env);

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

            QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            dataDir = "Z:" + dataDir.replace('/', '\\');

            QJsonObject startInfo;
            startInfo["WorkingDirectory"] = dataDir;
            startInfo["ConfigurationPath"] = dataDir + "\\dalamudConfig.json";
            startInfo["PluginDirectory"] = dataDir + "\\installedPlugins";
            startInfo["AssetDirectory"] = dataDir + "\\DalamudAssets";
            startInfo["DefaultPluginDirectory"] = dataDir + "\\devPlugins";
            startInfo["DelayInitializeMs"] = 0;
            startInfo["GameVersion"] = profile.gameVersion;
            startInfo["Language"] = profile.language;
            startInfo["OptOutMbCollection"] = false;

            QString argsEncoded = QJsonDocument(startInfo).toJson().toBase64();

            auto dalamudProcess = new QProcess();
            dalamudProcess->setProcessChannelMode(QProcess::ForwardedChannels);

            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert("DALAMUD_RUNTIME", dataDir + "\\DalamudRuntime");

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
            env.insert("XL_WINEONLINUX", "true");
#endif
            dalamudProcess->setProcessEnvironment(env);

            auto list = dalamudProcess->processEnvironment().toStringList();

            launchExecutable(profile, dalamudProcess, {dataDir + "/Dalamud/" + "Dalamud.Injector.exe", output, argsEncoded});
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

    connect(gameProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){
                gameClosed();
            });

    launchGameExecutable(profile, gameProcess, arguments);

    successfulLaunch();
}

void LauncherCore::launchExecutable(const ProfileSettings& profile, const QStringList args) {
    auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(profile, process, args);
}

void LauncherCore::launchGameExecutable(const ProfileSettings& profile, QProcess* process, const QStringList args) {
    QList<QString> arguments;

    arguments.append(args);

    launchExecutable(profile, process, arguments);
}

void LauncherCore::launchExecutable(const ProfileSettings& profile, QProcess* process, const QStringList args) {
    QList<QString> arguments;
    auto env = process->processEnvironment();

#if defined(Q_OS_LINUX)
    if(profile.useGamescope) {
        arguments.push_back("gamescope");

        if(profile.gamescope.fullscreen)
            arguments.push_back("-f");

        if(profile.gamescope.borderless)
            arguments.push_back("-b");

        if(profile.gamescope.width > 0)
            arguments.push_back("-w " + QString::number(profile.gamescope.width));

        if(profile.gamescope.height > 0)
            arguments.push_back("-h " + QString::number(profile.gamescope.height));

        if(profile.gamescope.refreshRate > 0)
            arguments.push_back("-r " + QString::number(profile.gamescope.refreshRate));
    }

    if(profile.useGamemode)
        arguments.push_back("gamemoderun");
#endif

#if defined(Q_OS_LINUX)
    if(profile.useEsync) {
        env.insert("WINEESYNC", QString::number(1));
        env.insert("WINEFSYNC", QString::number(1));
        env.insert("WINEFSYNC_FUTEX2", QString::number(1));
    }
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env.insert("WINEPREFIX", profile.winePrefixPath);

    arguments.push_back(profile.winePath);
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    process->setWorkingDirectory(profile.gamePath + "/game/");
    process->setProcessEnvironment(env);

    qDebug() << "launching " << executable << " with args" << arguments;

    process->start(executable, arguments);
}

QString LauncherCore::readVersion(QString path) {
    QFile file(path);
    file.open(QFile::OpenModeFlag::ReadOnly);

    return file.readAll();
}

void LauncherCore::readInitialInformation() {
    defaultProfileIndex = settings.value("defaultProfile", 0).toInt();

    appSettings.closeWhenLaunched = settings.value("closeWhenLaunched", true).toBool();

    gamescopeAvailable = checkIfInPath("gamescope");
    gamemodeAvailable = checkIfInPath("gamemoderun");

    const QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const bool hasDalamud = QFile::exists(dataDir + "/Dalamud");
    if (hasDalamud) {
        if (QFile::exists(dataDir + "/Dalamud/Dalamud.deps.json")) {
            QFile depsJson(dataDir + "/Dalamud/Dalamud.deps.json");
            depsJson.open(QFile::ReadOnly);
            QJsonDocument doc = QJsonDocument::fromJson(depsJson.readAll());

            // TODO: UGLY
            QString versionString =
                doc["targets"]
                    .toObject()[".NETCoreApp,Version=v5.0"]
                    .toObject()
                    .keys()
                    .filter("Dalamud")[0];
            dalamudVersion = versionString.remove("Dalamud/");
        }

        if(QFile::exists(dataDir + "/DalamudAssets/asset.ver")) {
            QFile assetJson(dataDir + "/DalamudAssets/asset.ver");
            assetJson.open(QFile::ReadOnly | QFile::Text);

            dalamudAssetVersion = QString(assetJson.readAll()).toInt();
        }

        if(QFile::exists(dataDir + "/DalamudRuntime/runtime.ver")) {
            QFile runtimeVer(dataDir + "/DalamudRuntime/runtime.ver");
            runtimeVer.open(QFile::ReadOnly | QFile::Text);

            runtimeVersion = QString(runtimeVer.readAll());
        }
    }

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
        profile.wineVersion = settings.value("wineVersion", getDefaultWineVersion()).toInt();

        readWineInfo(profile);

        if(settings.contains("gamePath") && settings.value("gamePath").canConvert<QString>() && !settings.value("gamePath").toString().isEmpty()) {
            profile.gamePath = settings.value("gamePath").toString();
        } else {
            profile.gamePath = getDefaultGamePath();
        }

        if(settings.contains("winePrefixPath") && settings.value("winePrefixPath").canConvert<QString>() && !settings.value("winePrefixPath").toString().isEmpty()) {
            profile.winePrefixPath = settings.value("winePrefixPath").toString();
        } else {
            profile.winePrefixPath = getDefaultWinePrefixPath();
        }

        ProfileSettings defaultSettings;

        // login
        profile.encryptArguments = settings.value("encryptArguments", defaultSettings.encryptArguments).toBool();
        profile.isSapphire = settings.value("isSapphire", defaultSettings.isSapphire).toBool();
        profile.lobbyURL = settings.value("lobbyURL", defaultSettings.lobbyURL).toString();
        profile.rememberUsername = settings.value("rememberUsername", defaultSettings.rememberUsername).toBool();
        profile.rememberPassword = settings.value("rememberPassword", defaultSettings.rememberPassword).toBool();
        profile.useSteam = settings.value("useSteam", defaultSettings.useSteam).toBool();

        profile.useDX9 = settings.value("useDX9", defaultSettings.useDX9).toBool();
        profile.useEsync = settings.value("useEsync", defaultSettings.useEsync).toBool();

        if(gamescopeAvailable)
            profile.useGamescope = settings.value("useGamescope", defaultSettings.useGamescope).toBool();

        if(gamemodeAvailable)
            profile.useGamemode = settings.value("useGamemode", defaultSettings.useGamemode).toBool();

        profile.enableDXVKhud = settings.value("enableDXVKhud", defaultSettings.enableDXVKhud).toBool();
        profile.enableWatchdog = settings.value("enableWatchdog", defaultSettings.enableWatchdog).toBool();

        // gamescope
        profile.gamescope.fullscreen = settings.value("gamescopeFullscreen", defaultSettings.gamescope.fullscreen).toBool();
        profile.gamescope.borderless = settings.value("gamescopeBorderless", defaultSettings.gamescope.borderless).toBool();
        profile.gamescope.width = settings.value("gamescopeWidth", defaultSettings.gamescope.width).toInt();
        profile.gamescope.height = settings.value("gamescopeHeight", defaultSettings.gamescope.height).toInt();
        profile.gamescope.refreshRate = settings.value("gamescopeRefreshRate", defaultSettings.gamescope.refreshRate).toInt();

        profile.enableDalamud = settings.value("enableDalamud", defaultSettings.enableDalamud).toBool();

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

    newProfile.wineVersion = getDefaultWineVersion();

    readWineInfo(newProfile);

    newProfile.gamePath = getDefaultGamePath();
    newProfile.winePrefixPath = getDefaultWinePrefixPath();

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
    settings.setValue("closeWhenLaunched", appSettings.closeWhenLaunched);

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

bool LauncherCore::checkIfInPath(const QString program) {
    // TODO: also check /usr/local/bin, /bin32 etc (basically read $PATH)
    const QString directory = "/usr/bin";

    QFileInfo fileInfo(directory + "/" + program);

    return fileInfo.exists() && fileInfo.isFile();
}

QString LauncherCore::getDefaultWinePrefixPath() {
#if defined(Q_OS_MACOS)
    return QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy";
#endif

#if defined(Q_OS_LINUX)
    return QDir::homePath() + "/.wine";
#endif
}

QString LauncherCore::getDefaultGamePath() {
#if defined(Q_OS_WIN)
    return "C:\\Program Files (x86)\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_MAC)
    return QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_LINUX)
    return QDir::homePath() + "/.wine/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif
}

int LauncherCore::getDefaultWineVersion() {
#if defined(Q_OS_MAC)
    return 2;
#else
    return 0;
#endif
}
