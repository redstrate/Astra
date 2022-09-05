#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenuBar>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <algorithm>
#include <utility>
#include <qt5keychain/keychain.h>
#include <cotp.h>

#ifdef USE_GAMEMODE
#include <gamemode_client.h>
#endif

#include "assetupdater.h"
#include "encryptedarg.h"
#include "launchercore.h"
#include "sapphirelauncher.h"
#include "squareboot.h"
#include "squarelauncher.h"

#ifdef ENABLE_WATCHDOG
    #include "watchdog.h"
#endif

void LauncherCore::setSSL(QNetworkRequest& request) {
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherCore::buildRequest(const ProfileSettings& settings, QNetworkRequest& request) {
    setSSL(request);

    if (settings.license == GameLicense::macOS) {
        request.setHeader(QNetworkRequest::UserAgentHeader, "macSQEXAuthor/2.0.0(MacOSX; ja-jp)");
    } else {
        request.setHeader(
            QNetworkRequest::UserAgentHeader,
            QString("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString(QSysInfo::bootUniqueId())));
    }

    request.setRawHeader(
        "Accept",
        "image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, "
        "application/x-ms-xbap, */*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept-Language", "en-us");
}

void LauncherCore::launchGame(const ProfileSettings& profile, const LoginAuth& auth) {
#ifdef ENABLE_WATCHDOG
    if (profile.enableWatchdog) {
        watchdog->launchGame(profile, auth);
    } else {
        beginGameExecutable(profile, auth);
    }
#else
    beginGameExecutable(profile, auth);
#endif
}

void LauncherCore::beginGameExecutable(const ProfileSettings& profile, const LoginAuth& auth) {
    QString gameExectuable;
    if(profile.useDX9) {
        gameExectuable = profile.gamePath + "/game/ffxiv.exe";
    } else {
        gameExectuable = profile.gamePath + "/game/ffxiv_dx11.exe";
    }

    if(profile.dalamud.enabled) {
        beginDalamudGame(gameExectuable, profile, auth);
    } else {
        beginVanillaGame(gameExectuable, profile, auth);
    }

    successfulLaunch();
}

void LauncherCore::beginVanillaGame(const QString& gameExecutablePath, const ProfileSettings& profile, const LoginAuth& auth) {
    auto gameProcess = new QProcess();
    gameProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    auto args = getGameArgs(profile, auth);

    launchExecutable(
        profile,
        gameProcess,
        {gameExecutablePath, args},
        true,
        true);
}

void LauncherCore::beginDalamudGame(const QString& gameExecutablePath, const ProfileSettings& profile, const LoginAuth& auth) {
    QString gamePath = gameExecutablePath;
    gamePath = "Z:" + gamePath.replace('/', '\\');

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dataDir = "Z:" + dataDir.replace('/', '\\');

    auto dalamudProcess = new QProcess();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DALAMUD_RUNTIME", dataDir + "\\DalamudRuntime");

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    env.insert("XL_WINEONLINUX", "true");
#endif
    dalamudProcess->setProcessEnvironment(env);

    auto args = getGameArgs(profile, auth);

    launchExecutable(
        profile,
        dalamudProcess,
        {dataDir + "/Dalamud/" + "Dalamud.Injector.exe", "launch", "-m", "inject",
            "--game=" + gamePath,
            "--dalamud-configuration-path=" + dataDir + "\\dalamudConfig.json",
            "--dalamud-plugin-directory=" + dataDir + "\\installedPlugins",
            "--dalamud-asset-directory=" + dataDir + "\\DalamudAssets",
            "--dalamud-client-language=" + QString::number(profile.language),
            "--", args},
        true,
        true);
}

QString LauncherCore::getGameArgs(const ProfileSettings& profile, const LoginAuth& auth) {
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
    gameArgs.push_back({"ver", profile.repositories.repositories[0].version});

    if (!auth.lobbyhost.isEmpty()) {
        gameArgs.push_back({"DEV.GMServerHost", auth.frontierHost});
        for (int i = 1; i < 9; i++) {
            gameArgs.push_back({QString("DEV.LobbyHost0%1").arg(QString::number(i)), auth.lobbyhost});
            gameArgs.push_back({QString("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
        }
    }

    if (profile.license == GameLicense::WindowsSteam) {
        gameArgs.push_back({"IsSteam", "1"});
    }

    const QString argFormat = profile.encryptArguments ? " /%1 =%2" : " %1=%2";

    QString argJoined;
    for (const auto& arg : gameArgs) {
        argJoined += argFormat.arg(arg.key, arg.value);
    }

    return profile.encryptArguments ? encryptGameArg(argJoined) : argJoined;
}

void LauncherCore::launchExecutable(
    const ProfileSettings& profile,
    QProcess* process,
    const QStringList& args,
    bool isGame,
    bool needsRegistrySetup) {
    QList<QString> arguments;
    auto env = process->processEnvironment();

    if (needsRegistrySetup) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        if (profile.license == GameLicense::macOS) {
            addRegistryKey(profile, "HKEY_CURRENT_USER\\Software\\Wine", "HideWineExports", "0");
        } else {
            addRegistryKey(profile, "HKEY_CURRENT_USER\\Software\\Wine", "HideWineExports", "1");
        }
#endif
    }

#if defined(Q_OS_LINUX)
    if (isGame) {
        if (profile.useGamescope) {
            arguments.push_back("gamescope");

            if (profile.gamescope.fullscreen)
                arguments.push_back("-f");

            if (profile.gamescope.borderless)
                arguments.push_back("-b");

            if (profile.gamescope.width > 0)
                arguments.push_back("-w " + QString::number(profile.gamescope.width));

            if (profile.gamescope.height > 0)
                arguments.push_back("-h " + QString::number(profile.gamescope.height));

            if (profile.gamescope.refreshRate > 0)
                arguments.push_back("-r " + QString::number(profile.gamescope.refreshRate));
        }
    }
#endif

#if USE_GAMEMODE
    if(isGame && profile.useGamemode) {
        gamemode_request_start();
    }
#endif

#if defined(Q_OS_LINUX)
    if (profile.useEsync) {
        env.insert("WINEESYNC", QString::number(1));
        env.insert("WINEFSYNC", QString::number(1));
        env.insert("WINEFSYNC_FUTEX2", QString::number(1));
    }
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env.insert("WINEPREFIX", profile.winePrefixPath);

    // XIV on Mac bundle their own Wine install directory, complete with libs etc
    if (profile.wineType == WineType::XIVOnMac) {
        // TODO: don't hardcode this
        QString xivLibPath = "/Applications/XIV on Mac.app/Contents/Resources/wine/lib:/Applications/XIV on "
                             "Mac.app/Contents/Resources/MoltenVK/modern";

        env.insert("DYLD_FALLBACK_LIBRARY_PATH", xivLibPath);
        env.insert("DYLD_VERSIONED_LIBRARY_PATH", xivLibPath);
        env.insert("MVK_CONFIG_FULL_IMAGE_VIEW_SWIZZLE", "1");
        env.insert("MVK_CONFIG_RESUME_LOST_DEVICE", "1");
        env.insert("MVK_ALLOW_METAL_FENCES", "1");
        env.insert("MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS", "1");
    }

    #if defined(FLATPAK)
    arguments.push_back("flatpak-spawn");
    arguments.push_back("--host");
    #endif

    arguments.push_back(profile.winePath);
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    if (isGame)
        process->setWorkingDirectory(profile.gamePath + "/game/");

    process->setProcessEnvironment(env);

    process->start(executable, arguments);
}

void LauncherCore::readInitialInformation() {
    defaultProfileIndex = settings.value("defaultProfile", 0).toInt();

    auto defaultAppSettings = AppSettings();
    appSettings.closeWhenLaunched = settings.value("closeWhenLaunched", defaultAppSettings.closeWhenLaunched).toBool();
    appSettings.showBanners = settings.value("showBanners", defaultAppSettings.showBanners).toBool();
    appSettings.showNewsList = settings.value("showNewsList", defaultAppSettings.showNewsList).toBool();

    gamescopeAvailable = checkIfInPath("gamescope");
    gamemodeAvailable = checkIfInPath("gamemoderun");

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const bool hasDalamud = QFile::exists(dataDir + "/Dalamud");
    if (hasDalamud) {
        if (QFile::exists(dataDir + "/Dalamud/Dalamud.deps.json")) {
            QFile depsJson(dataDir + "/Dalamud/Dalamud.deps.json");
            depsJson.open(QFile::ReadOnly);
            QJsonDocument doc = QJsonDocument::fromJson(depsJson.readAll());

            QString versionString;
            if (doc["targets"].toObject().contains(".NETCoreApp,Version=v5.0")) {
                versionString =
                    doc["targets"].toObject()[".NETCoreApp,Version=v5.0"].toObject().keys().filter("Dalamud")[0];
            } else {
                versionString =
                    doc["targets"].toObject()[".NETCoreApp,Version=v6.0"].toObject().keys().filter("Dalamud")[0];
            }

            dalamudVersion = versionString.remove("Dalamud/");
        }

        if (QFile::exists(dataDir + "/DalamudAssets/asset.ver")) {
            QFile assetJson(dataDir + "/DalamudAssets/asset.ver");
            assetJson.open(QFile::ReadOnly | QFile::Text);

            dalamudAssetVersion = QString(assetJson.readAll()).toInt();
        }

        if (QFile::exists(dataDir + "/DalamudRuntime/runtime.ver")) {
            QFile runtimeVer(dataDir + "/DalamudRuntime/runtime.ver");
            runtimeVer.open(QFile::ReadOnly | QFile::Text);

            runtimeVersion = QString(runtimeVer.readAll());
        }
    }

    auto profiles = settings.childGroups();

    // create the Default profile if it doesnt exist
    if (profiles.empty())
        profiles.append(QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces));

    profileSettings.resize(profiles.size());

    for (const auto& uuid : profiles) {
        auto profile = new ProfileSettings();
        profile->uuid = QUuid(uuid);

        settings.beginGroup(uuid);

        profile->name = settings.value("name", "Default").toString();

        if (settings.contains("gamePath") && settings.value("gamePath").canConvert<QString>() &&
            !settings.value("gamePath").toString().isEmpty()) {
            profile->gamePath = settings.value("gamePath").toString();
        } else {
            profile->gamePath = getDefaultGamePath();
        }

        if (settings.contains("winePrefixPath") && settings.value("winePrefixPath").canConvert<QString>() &&
            !settings.value("winePrefixPath").toString().isEmpty()) {
            profile->winePrefixPath = settings.value("winePrefixPath").toString();
        } else {
            profile->winePrefixPath = getDefaultWinePrefixPath();
        }

        if (settings.contains("winePath") && settings.value("winePath").canConvert<QString>() &&
            !settings.value("winePath").toString().isEmpty()) {
            profile->winePath = settings.value("winePath").toString();
        }

        ProfileSettings defaultSettings;

        // login
        profile->encryptArguments = settings.value("encryptArguments", defaultSettings.encryptArguments).toBool();
        profile->isSapphire = settings.value("isSapphire", defaultSettings.isSapphire).toBool();
        profile->lobbyURL = settings.value("lobbyURL", defaultSettings.lobbyURL).toString();
        profile->rememberUsername = settings.value("rememberUsername", defaultSettings.rememberUsername).toBool();
        profile->rememberPassword = settings.value("rememberPassword", defaultSettings.rememberPassword).toBool();
        profile->rememberOTPSecret = settings.value("rememberOTPSecret", defaultSettings.rememberOTPSecret).toBool();
        profile->useOneTimePassword = settings.value("useOneTimePassword", defaultSettings.useOneTimePassword).toBool();
        profile->license = (GameLicense)settings.value("license", (int)defaultSettings.license).toInt();
        profile->isFreeTrial = settings.value("isFreeTrial", defaultSettings.isFreeTrial).toBool();
        profile->autoLogin = settings.value("autoLogin", defaultSettings.autoLogin).toBool();

        profile->useDX9 = settings.value("useDX9", defaultSettings.useDX9).toBool();

        // wine
        profile->wineType = (WineType)settings.value("wineType", (int)defaultSettings.wineType).toInt();
        profile->useEsync = settings.value("useEsync", defaultSettings.useEsync).toBool();

        readWineInfo(*profile);

        if (gamescopeAvailable)
            profile->useGamescope = settings.value("useGamescope", defaultSettings.useGamescope).toBool();

        if (gamemodeAvailable)
            profile->useGamemode = settings.value("useGamemode", defaultSettings.useGamemode).toBool();

        profile->enableDXVKhud = settings.value("enableDXVKhud", defaultSettings.enableDXVKhud).toBool();

        profile->enableWatchdog = settings.value("enableWatchdog", defaultSettings.enableWatchdog).toBool();

        // gamescope
        profile->gamescope.borderless =
            settings.value("gamescopeBorderless", defaultSettings.gamescope.borderless).toBool();
        profile->gamescope.width = settings.value("gamescopeWidth", defaultSettings.gamescope.width).toInt();
        profile->gamescope.height = settings.value("gamescopeHeight", defaultSettings.gamescope.height).toInt();
        profile->gamescope.refreshRate =
            settings.value("gamescopeRefreshRate", defaultSettings.gamescope.refreshRate).toInt();

        profile->dalamud.enabled = settings.value("enableDalamud", defaultSettings.dalamud.enabled).toBool();
        profile->dalamud.optOutOfMbCollection =
            settings.value("dalamudOptOut", defaultSettings.dalamud.optOutOfMbCollection).toBool();
        profile->dalamud.channel =
            (DalamudChannel)settings.value("dalamudChannel", (int)defaultSettings.dalamud.channel).toInt();

        profileSettings[settings.value("index").toInt()] = profile;

        settings.endGroup();
    }

    readGameVersion();
}

void LauncherCore::readWineInfo(ProfileSettings& profile) {
#if defined(Q_OS_MAC)
    switch (profile.wineType) {
        case WineType::System: // system wine
            profile.winePath = "/usr/local/bin/wine64";
            break;
        case WineType::Custom: // custom path
            profile.winePath = profile.winePath;
            break;
        case WineType::Builtin: // ffxiv built-in (for mac users)
            profile.winePath = "/Applications/FINAL FANTASY XIV "
                               "ONLINE.app/Contents/SharedSupport/finalfantasyxiv/FINAL FANTASY XIV ONLINE/wine";
            break;
        case WineType::XIVOnMac:
            profile.winePath = "/Applications/XIV on Mac.app/Contents/Resources/wine/bin/wine64";
            break;
    }
#endif

#if defined(Q_OS_LINUX)
    switch (profile.wineType) {
        case WineType::System: // system wine (should be in $PATH)
            profile.winePath = "/usr/bin/wine";
            break;
        case WineType::Custom: // custom pth
            profile.winePath = profile.winePath;
            break;
    }
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    auto wineProcess = new QProcess(this);
    wineProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(wineProcess, &QProcess::readyRead, this, [wineProcess, &profile] {
        profile.wineVersion = wineProcess->readAllStandardOutput().trimmed();
    });

    launchExecutable(profile, wineProcess, {"--version"}, false, false);

    wineProcess->waitForFinished();
#endif
}

void LauncherCore::readGameVersion() {
    for (auto& profile : profileSettings) {
        profile->gameData = physis_gamedata_initialize((profile->gamePath + "/game").toStdString().c_str());
        profile->bootData = physis_bootdata_initialize((profile->gamePath + "/boot").toStdString().c_str());

        profile->repositories = physis_gamedata_get_repositories(profile->gameData);
        profile->bootVersion = physis_bootdata_get_version(profile->bootData);

        readGameData(*profile);
    }
}

LauncherCore::LauncherCore()
    : settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName()) {
    mgr = new QNetworkAccessManager();
    sapphireLauncher = new SapphireLauncher(*this);
    squareLauncher = new SquareLauncher(*this);
    squareBoot = new SquareBoot(*this, *squareLauncher);
    assetUpdater = new AssetUpdater(*this);
    steamApi = new SteamAPI();

#ifdef ENABLE_WATCHDOG
    watchdog = new Watchdog(*this);
#endif

    readInitialInformation();
}

ProfileSettings& LauncherCore::getProfile(int index) {
    return *profileSettings[index];
}

int LauncherCore::getProfileIndex(const QString& name) {
    for (int i = 0; i < profileSettings.size(); i++) {
        if (profileSettings[i]->name == name)
            return i;
    }

    return -1;
}

QList<QString> LauncherCore::profileList() const {
    QList<QString> list;
    for (auto profile : profileSettings) {
        list.append(profile->name);
    }

    return list;
}

int LauncherCore::addProfile() {
    auto newProfile = new ProfileSettings();
    newProfile->uuid = QUuid::createUuid();
    newProfile->name = "New Profile";

    readWineInfo(*newProfile);

    newProfile->gamePath = getDefaultGamePath();
    newProfile->winePrefixPath = getDefaultWinePrefixPath();

    profileSettings.append(newProfile);

    settingsChanged();

    return profileSettings.size() - 1;
}

int LauncherCore::deleteProfile(const QString& name) {
    int index = 0;
    for (int i = 0; i < profileSettings.size(); i++) {
        if (profileSettings[i]->name == name)
            index = i;
    }

    // remove group so it doesnt stay
    settings.beginGroup(profileSettings[index]->uuid.toString(QUuid::StringFormat::WithoutBraces));
    settings.remove("");
    settings.endGroup();

    profileSettings.removeAt(index);

    return index - 1;
}

void LauncherCore::saveSettings() {
    settings.setValue("defaultProfile", defaultProfileIndex);
    settings.setValue("closeWhenLaunched", appSettings.closeWhenLaunched);
    settings.setValue("showBanners", appSettings.showBanners);
    settings.setValue("showNewsList", appSettings.showNewsList);

    for (int i = 0; i < profileSettings.size(); i++) {
        const auto& profile = profileSettings[i];

        settings.beginGroup(profile->uuid.toString(QUuid::StringFormat::WithoutBraces));

        settings.setValue("name", profile->name);
        settings.setValue("index", i);

        // game
        settings.setValue("useDX9", profile->useDX9);
        settings.setValue("gamePath", profile->gamePath);

        // wine
        settings.setValue("wineType", (int)profile->wineType);
        settings.setValue("winePath", profile->winePath);
        settings.setValue("winePrefixPath", profile->winePrefixPath);

        settings.setValue("useEsync", profile->useEsync);
        settings.setValue("useGamescope", profile->useGamescope);
        settings.setValue("useGamemode", profile->useGamemode);

        // gamescope
        settings.setValue("gamescopeFullscreen", profile->gamescope.fullscreen);
        settings.setValue("gamescopeBorderless", profile->gamescope.borderless);
        settings.setValue("gamescopeWidth", profile->gamescope.width);
        settings.setValue("gamescopeHeight", profile->gamescope.height);
        settings.setValue("gamescopeRefreshRate", profile->gamescope.refreshRate);

        // login
        settings.setValue("encryptArguments", profile->encryptArguments);
        settings.setValue("isSapphire", profile->isSapphire);
        settings.setValue("lobbyURL", profile->lobbyURL);
        settings.setValue("rememberUsername", profile->rememberUsername);
        settings.setValue("rememberPassword", profile->rememberPassword);
        settings.setValue("rememberOTPSecret", profile->rememberOTPSecret);
        settings.setValue("useOneTimePassword", profile->useOneTimePassword);
        settings.setValue("license", (int)profile->license);
        settings.setValue("isFreeTrial", profile->isFreeTrial);
        settings.setValue("autoLogin", profile->autoLogin);

        settings.setValue("enableDalamud", profile->dalamud.enabled);
        settings.setValue("dalamudOptOut", profile->dalamud.optOutOfMbCollection);
        settings.setValue("dalamudChannel", (int)profile->dalamud.channel);
        settings.setValue("enableWatchdog", profile->enableWatchdog);

        settings.endGroup();
    }
}

bool LauncherCore::checkIfInPath(const QString& program) {
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

    return "";
}

QString LauncherCore::getDefaultGamePath() {
#if defined(Q_OS_WIN)
    return "C:\\Program Files (x86)\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_MAC)
    return QDir::homePath() +
           "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program "
           "Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_LINUX)
    return QDir::homePath() + "/.wine/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif
}

void LauncherCore::addRegistryKey(const ProfileSettings& settings, QString key, QString value, QString data) {
    auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(settings, process, {"reg", "add", std::move(key), "/v", value, "/d", data, "/f"}, false, false);
}

void LauncherCore::readGameData(ProfileSettings& profile) {
    EXH* exh = physis_gamedata_read_excel_sheet_header(profile.gameData, "ExVersion");
    if (exh != nullptr) {
        physis_EXD exd = physis_gamedata_read_excel_sheet(profile.gameData, "ExVersion", exh, Language::English, 0);

        for (int i = 0; i < exd.row_count; i++) {
            expansionNames.push_back(exd.row_data[i].column_data[0].string._0);
        }

        physis_gamedata_free_sheet(exd);
        physis_gamedata_free_sheet_header(exh);
    }
}

void LauncherCore::login(LoginInformation& loginInformation) {
    if (loginInformation.settings->isSapphire) {
        sapphireLauncher->login(loginInformation.settings->lobbyURL, loginInformation);
    } else {
        squareBoot->checkGateStatus(&loginInformation);
    }
}

bool LauncherCore::autoLogin(ProfileSettings& profile) {
    auto loop = new QEventLoop();

    QString username, password;
    QString otpSecret;

    auto usernameJob = new QKeychain::ReadPasswordJob("LauncherWindow");
    usernameJob->setKey(profile.name + "-username");
    usernameJob->start();

    QObject::connect(
        usernameJob, &QKeychain::ReadPasswordJob::finished, [loop, usernameJob, &username](QKeychain::Job* j) {
            username = usernameJob->textData();
            loop->quit();
        });

    loop->exec();

    auto passwordJob = new QKeychain::ReadPasswordJob("LauncherWindow");
    passwordJob->setKey(profile.name + "-password");
    passwordJob->start();

    QObject::connect(
        passwordJob, &QKeychain::ReadPasswordJob::finished, [loop, passwordJob, &password](QKeychain::Job* j) {
            password = passwordJob->textData();
            loop->quit();
        });

    loop->exec();

    // TODO: handle cases where the user doesn't want to store their OTP secret, so we have to manually prompt them
    if(profile.useOneTimePassword && profile.rememberOTPSecret) {
        auto otpJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        otpJob->setKey(profile.name + "-otpsecret");
        otpJob->start();

        QObject::connect(
            otpJob, &QKeychain::ReadPasswordJob::finished, [loop, otpJob, &otpSecret](QKeychain::Job* j) {
                otpSecret = otpJob->textData();
                loop->quit();
            });

        loop->exec();
    }

    auto info = new LoginInformation();
    info->settings = &profile;
    info->username = username;
    info->password = password;

    if(profile.useOneTimePassword && !profile.rememberOTPSecret)
        return false;

    if(profile.useOneTimePassword && profile.rememberOTPSecret) {
        // generate otp
        char* totp = get_totp (otpSecret.toStdString().c_str(), 6, 30, SHA1, nullptr);
        info->oneTimePassword = totp;
        free (totp);
    }

    // TODO: when login fails, we need some way to propagate this back? or not?
    login(*info);

    return true;
}
