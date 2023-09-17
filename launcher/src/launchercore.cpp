// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QStandardPaths>
#include <algorithm>
#include <qcoronetworkreply.h>
#include <utility>

#ifdef ENABLE_GAMEMODE
#include <gamemode_client.h>
#endif

#include "account.h"
#include "assetupdater.h"
#include "compatibilitytoolinstaller.h"
#include "config.h"
#include "encryptedarg.h"
#include "launchercore.h"
#include "sapphirelauncher.h"
#include "squarelauncher.h"
#include "utility.h"

#ifdef ENABLE_WATCHDOG
#include "watchdog.h"
#endif

void LauncherCore::setSSL(QNetworkRequest &request)
{
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherCore::buildRequest(const Profile &settings, QNetworkRequest &request)
{
    setSSL(request);

    if (settings.account()->license() == Account::GameLicense::macOS) {
        request.setHeader(QNetworkRequest::UserAgentHeader, "macSQEXAuthor/2.0.0(MacOSX; ja-jp)");
    } else {
        request.setHeader(QNetworkRequest::UserAgentHeader, QString("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString(QSysInfo::bootUniqueId())));
    }

    request.setRawHeader("Accept",
                         "image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, "
                         "application/x-ms-xbap, */*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept-Language", "en-us");
}

void LauncherCore::launchGame(const Profile &profile, const LoginAuth &auth)
{
    steamApi->setLauncherMode(false);

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

void LauncherCore::beginGameExecutable(const Profile &profile, const LoginAuth &auth)
{
    Q_EMIT stageChanged("Launching game...");

    QString gameExectuable;
    if (profile.directx9Enabled()) {
        gameExectuable = profile.gamePath() + "/game/ffxiv.exe";
    } else {
        gameExectuable = profile.gamePath() + "/game/ffxiv_dx11.exe";
    }

    if (profile.dalamudEnabled()) {
        beginDalamudGame(gameExectuable, profile, auth);
    } else {
        beginVanillaGame(gameExectuable, profile, auth);
    }

    successfulLaunch();
}

void LauncherCore::beginVanillaGame(const QString &gameExecutablePath, const Profile &profile, const LoginAuth &auth)
{
    auto gameProcess = new QProcess(this);
    gameProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    connect(gameProcess, &QProcess::finished, this, [this](int exitCode) {
        Q_UNUSED(exitCode)
        Q_EMIT gameClosed();
    });

    auto args = getGameArgs(profile, auth);

    launchExecutable(profile, gameProcess, {gameExecutablePath, args}, true, true);
}

void LauncherCore::beginDalamudGame(const QString &gameExecutablePath, const Profile &profile, const LoginAuth &auth)
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QDir stateDir = Utility::stateDirectory();
    const QDir dalamudDir = dataDir.absoluteFilePath("dalamud");

    const QDir dalamudConfigDir = configDir.absoluteFilePath("dalamud");
    const QDir userDalamudConfigDir = dalamudConfigDir.absoluteFilePath(profile.account()->uuid());

    const QDir dalamudBasePluginDir = dalamudDir.absoluteFilePath("plugins");
    const QDir dalamudUserPluginDir = dalamudBasePluginDir.absoluteFilePath(profile.account()->uuid());

    // Some really dumb plugins check for "installedPlugins" in their assembly directory FOR SOME REASON
    // so we need to match typical XIVQuickLauncher behavior here. Why? I have no clue.
    const QDir dalamudPluginDir = dalamudUserPluginDir.absoluteFilePath("installedPlugins");

    const QString logDir = stateDir.absoluteFilePath("logs");

    if (!QDir().exists(logDir))
        QDir().mkpath(logDir);

    const QDir dalamudRuntimeDir = dalamudDir.absoluteFilePath("runtime");
    const QDir dalamudAssetDir = dalamudDir.absoluteFilePath("assets");
    const QDir dalamudConfigPath = userDalamudConfigDir.absoluteFilePath("dalamud-config.json");

    const QDir dalamudInstallDir = dalamudDir.absoluteFilePath(profile.dalamudChannelName());
    const QString dalamudInjector = dalamudInstallDir.absoluteFilePath("Dalamud.Injector.exe");

    auto dalamudProcess = new QProcess(this);
    connect(dalamudProcess, &QProcess::finished, this, [this](int exitCode) {
        Q_UNUSED(exitCode)
        Q_EMIT gameClosed();
    });

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DALAMUD_RUNTIME", Utility::toWindowsPath(dalamudRuntimeDir));

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    env.insert("XL_WINEONLINUX", "true");
#endif
    dalamudProcess->setProcessEnvironment(env);

    auto args = getGameArgs(profile, auth);

    launchExecutable(profile,
                     dalamudProcess,
                     {Utility::toWindowsPath(dalamudInjector),
                      "launch",
                      "-m",
                      "inject",
                      "--game=" + Utility::toWindowsPath(gameExecutablePath),
                      "--dalamud-configuration-path=" + Utility::toWindowsPath(dalamudConfigPath),
                      "--dalamud-plugin-directory=" + Utility::toWindowsPath(dalamudPluginDir),
                      "--dalamud-asset-directory=" + Utility::toWindowsPath(dalamudAssetDir),
                      "--dalamud-client-language=" + QString::number(profile.account()->language()),
                      "--logpath=" + Utility::toWindowsPath(logDir),
                      "--",
                      args},
                     true,
                     true);
}

QString LauncherCore::getGameArgs(const Profile &profile, const LoginAuth &auth)
{
    struct Argument {
        QString key, value;
    };

    QList<Argument> gameArgs;
    gameArgs.push_back({"DEV.DataPathType", QString::number(1)});
    gameArgs.push_back({"DEV.UseSqPack", QString::number(1)});

    gameArgs.push_back({"DEV.MaxEntitledExpansionID", QString::number(auth.maxExpansion)});
    gameArgs.push_back({"DEV.TestSID", auth.SID});
    gameArgs.push_back({"SYS.Region", QString::number(auth.region)});
    gameArgs.push_back({"language", QString::number(profile.account()->language())});
    gameArgs.push_back({"ver", profile.repositories.repositories[0].version});
    gameArgs.push_back({"UserPath", Utility::toWindowsPath(profile.account()->getConfigDir().absolutePath())});

    // FIXME: this should belong somewhere else...
    if (!QDir().exists(profile.account()->getConfigDir().absolutePath())) {
        QDir().mkpath(profile.account()->getConfigDir().absolutePath());
    }

    if (!auth.lobbyhost.isEmpty()) {
        gameArgs.push_back({"DEV.GMServerHost", auth.frontierHost});
        for (int i = 1; i < 9; i++) {
            gameArgs.push_back({QString("DEV.LobbyHost0%1").arg(QString::number(i)), auth.lobbyhost});
            gameArgs.push_back({QString("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
        }
    }

    if (profile.account()->license() == Account::GameLicense::WindowsSteam) {
        gameArgs.push_back({"IsSteam", "1"});
    }

    const QString argFormat = profile.argumentsEncrypted() ? " /%1 =%2" : " %1=%2";

    QString argJoined;
    for (const auto &arg : gameArgs) {
        argJoined += argFormat.arg(arg.key, arg.value);
    }

    return profile.argumentsEncrypted() ? encryptGameArg(argJoined) : argJoined;
}

void LauncherCore::launchExecutable(const Profile &profile, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup)
{
    QList<QString> arguments;
    auto env = process->processEnvironment();

    if (needsRegistrySetup) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        if (profile.account()->license() == Account::GameLicense::macOS) {
            addRegistryKey(profile, "HKEY_CURRENT_USER\\Software\\Wine", "HideWineExports", "0");
        } else {
            addRegistryKey(profile, "HKEY_CURRENT_USER\\Software\\Wine", "HideWineExports", "1");
        }
#endif
    }

#if defined(Q_OS_LINUX)
    if (isGame) {
        if (profile.gamescopeEnabled()) {
            arguments.push_back("gamescope");

            if (profile.gamescopeFullscreen())
                arguments.push_back("-f");

            if (profile.gamescopeBorderless())
                arguments.push_back("-b");

            if (profile.gamescopeWidth() > 0)
                arguments.push_back("-w " + QString::number(profile.gamescopeWidth()));

            if (profile.gamescopeHeight() > 0)
                arguments.push_back("-h " + QString::number(profile.gamescopeHeight()));

            if (profile.gamescopeRefreshRate() > 0)
                arguments.push_back("-r " + QString::number(profile.gamescopeRefreshRate()));
        }
    }
#endif

#ifdef ENABLE_GAMEMODE
    if (isGame && profile.gamemodeEnabled()) {
        gamemode_request_start();
    }
#endif

#if defined(Q_OS_LINUX)
    if (profile.esyncEnabled()) {
        env.insert("WINEESYNC", QString::number(1));
        env.insert("WINEFSYNC", QString::number(1));
        env.insert("WINEFSYNC_FUTEX2", QString::number(1));
    }
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    if (m_isSteam) {
        const QDir steamDirectory = QProcessEnvironment::systemEnvironment().value("STEAM_COMPAT_CLIENT_INSTALL_PATH");
        const QDir compatData =
            QProcessEnvironment::systemEnvironment().value("STEAM_COMPAT_DATA_PATH"); // TODO: do these have to exist on the root steam folder?

        const QString steamAppsPath = steamDirectory.absoluteFilePath("steamapps/common");

        // Find the highest Proton version
        QDirIterator it(steamAppsPath);
        QDir highestVersion;
        int highestVersionNum = 1;
        while (it.hasNext()) {
            QString dir = it.next();

            QFileInfo fileInfo(dir);
            if (!fileInfo.isDir()) {
                continue;
            }

            QString dirName = fileInfo.fileName();
            if (dirName.contains("Proton")) {
                if (dirName == "Proton - Experimental") {
                    highestVersion.setPath(dir);
                    break;
                } else {
                    QString version = dirName.remove("Proton ");
                    // Exclude "BattlEye Runtime" and other unrelated things
                    if (version.contains('.')) {
                        // TODO: better error handling (they might never be invalid, but better safe than sorry)
                        QStringList parts = version.split('.');
                        int versionNum = parts[0].toInt();

                        // TODO: doesn't handle minor versions, not like they really exist anymore anyway
                        if (versionNum > highestVersionNum) {
                            highestVersionNum = versionNum;
                            highestVersion.setPath(dir);
                        }
                    }
                }
            }
        }

        env.insert("STEAM_COMPAT_CLIENT_INSTALL_PATH", steamDirectory.absolutePath());
        env.insert("STEAM_COMPAT_DATA_PATH", compatData.absolutePath());

        arguments.push_back(highestVersion.absoluteFilePath("proton"));
        arguments.push_back("run");

        qInfo() << arguments << env.toStringList();
    } else {
        env.insert("WINEPREFIX", profile.winePrefixPath());

        // XIV on Mac bundle their own Wine install directory, complete with libs etc
        if (profile.wineType() == Profile::WineType::XIVOnMac) {
            // TODO: don't hardcode this
            QString xivLibPath =
                "/Applications/XIV on Mac.app/Contents/Resources/wine/lib:/Applications/XIV on "
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
        arguments.push_back(profile.winePath());
    }
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    if (isGame)
        process->setWorkingDirectory(profile.gamePath() + "/game/");

    process->setProcessEnvironment(env);
    process->setProcessChannelMode(QProcess::ProcessChannelMode::ForwardedChannels);

    process->start(executable, arguments);
}

void LauncherCore::readInitialInformation()
{
    gamescopeAvailable = checkIfInPath("gamescope");
    gamemodeAvailable = checkIfInPath("gamemoderun");

    m_profileManager->load();
    m_accountManager->load();

    // restore profile -> account connections
    for (auto profile : m_profileManager->profiles()) {
        if (auto account = m_accountManager->getByUuid(profile->accountUuid())) {
            profile->setAccount(account);
        }
    }

    m_loadingFinished = true;
    Q_EMIT loadingFinished();
}

LauncherCore::LauncherCore()
{
    mgr = new QNetworkAccessManager(this);
    sapphireLauncher = new SapphireLauncher(*this, this);
    squareLauncher = new SquareLauncher(*this, this);
    squareBoot = new SquareBoot(*this, *squareLauncher, this);
    steamApi = new SteamAPI(*this, this);
    m_profileManager = new ProfileManager(*this, this);
    m_accountManager = new AccountManager(*this, this);

#ifdef ENABLE_WATCHDOG
    watchdog = new Watchdog(*this);
#endif

    readInitialInformation();

    steamApi->setLauncherMode(true);
}

bool LauncherCore::checkIfInPath(const QString &program)
{
    const auto pathList = qgetenv("PATH").split(':');

    return std::any_of(pathList.cbegin(), pathList.cend(), [program](const QByteArray &path) {
        QFileInfo fileInfo(path + "/" + program);
        return fileInfo.exists() && fileInfo.isFile();
    });
}

void LauncherCore::addRegistryKey(const Profile &settings, QString key, QString value, QString data)
{
    auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(settings, process, {"reg", "add", std::move(key), "/v", std::move(value), "/d", std::move(data), "/f"}, false, false);
}

void LauncherCore::login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword)
{
    qDebug() << "Logging in, performing asset update check.";

    auto assetUpdater = new AssetUpdater(*profile, *this, this);
    connect(assetUpdater, &AssetUpdater::finishedUpdating, this, [this, assetUpdater, profile, username, password, oneTimePassword] {
        qDebug() << "Assets done updating!";

        auto loginInformation = new LoginInformation(this);
        loginInformation->profile = profile;
        loginInformation->username = username;
        loginInformation->password = password;
        loginInformation->oneTimePassword = oneTimePassword;

        if (profile->account()->rememberPassword()) {
            profile->account()->setPassword(password);
        }

        if (loginInformation->profile->account()->isSapphire()) {
            sapphireLauncher->login(loginInformation->profile->account()->lobbyUrl(), *loginInformation);
        } else {
            squareBoot->checkGateStatus(loginInformation);
        }

        assetUpdater->deleteLater();
    });
    assetUpdater->update();
}

bool LauncherCore::autoLogin(Profile &profile)
{
    // TODO: when login fails, we need some way to propagate this back? or not?
    login(&profile, profile.account()->name(), profile.account()->getPassword(), profile.account()->getOTP());

    return true;
}

GameInstaller *LauncherCore::createInstaller(Profile *profile)
{
    Q_ASSERT(profile != nullptr);
    return new GameInstaller(*this, *profile, this);
}

CompatibilityToolInstaller *LauncherCore::createCompatInstaller()
{
    return new CompatibilityToolInstaller(*this, this);
}

bool LauncherCore::isLoadingFinished() const
{
    return m_loadingFinished;
}

bool LauncherCore::hasAccount() const
{
    return false;
}

ProfileManager *LauncherCore::profileManager()
{
    return m_profileManager;
}

AccountManager *LauncherCore::accountManager()
{
    return m_accountManager;
}

bool LauncherCore::closeWhenLaunched() const
{
    return Config::closeWhenLaunched();
}

void LauncherCore::setCloseWhenLaunched(const bool value)
{
    if (value != Config::closeWhenLaunched()) {
        Config::setCloseWhenLaunched(value);
        Config::self()->save();
        Q_EMIT closeWhenLaunchedChanged();
    }
}

bool LauncherCore::showNews() const
{
    return Config::showNews();
}

void LauncherCore::setShowNews(const bool value)
{
    if (value != Config::showNews()) {
        Config::setShowNews(value);
        Config::self()->save();
        Q_EMIT showNewsChanged();
    }
}

bool LauncherCore::showDevTools() const
{
    return Config::showDevTools();
}

void LauncherCore::setShowDevTools(const bool value)
{
    if (value != Config::showDevTools()) {
        Config::setShowDevTools(value);
        Config::self()->save();
        Q_EMIT showDevToolsChanged();
    }
}

bool LauncherCore::keepPatches() const
{
    return Config::keepPatches();
}

void LauncherCore::setKeepPatches(const bool value)
{
    if (value != Config::keepPatches()) {
        Config::setKeepPatches(value);
        Config::self()->save();
        Q_EMIT keepPatchesChanged();
    }
}

QString LauncherCore::dalamudDistribServer() const
{
    return Config::dalamudDistribServer();
}

void LauncherCore::setDalamudDistribServer(const QString &value)
{
    if (value != Config::dalamudDistribServer()) {
        Config::setDalamudDistribServer(value);
        Config::self()->save();
        Q_EMIT dalamudDistribServerChanged();
    }
}

QString LauncherCore::squareEnixServer() const
{
    return Config::squareEnixServer();
}

void LauncherCore::setSquareEnixServer(const QString &value)
{
    if (value != Config::squareEnixServer()) {
        Config::setSquareEnixServer(value);
        Config::self()->save();
        Q_EMIT squareEnixServerChanged();
    }
}

QString LauncherCore::squareEnixLoginServer() const
{
    return Config::squareEnixLoginServer();
}

void LauncherCore::setSquareEnixLoginServer(const QString &value)
{
    if (value != Config::squareEnixLoginServer()) {
        Config::setSquareEnixLoginServer(value);
        Config::self()->save();
        Q_EMIT squareEnixLoginServerChanged();
    }
}

void LauncherCore::refreshNews()
{
    fetchNews();
}

QCoro::Task<> LauncherCore::fetchNews()
{
    QUrlQuery query;
    query.addQueryItem("lang", "en-us");
    query.addQueryItem("media", "pcapp");

    QUrl url;
    url.setScheme("https");
    url.setHost(QStringLiteral("frontier.%1").arg(squareEnixServer()));
    url.setPath("/news/headline.json");
    url.setQuery(query);

    QNetworkRequest request(QString("%1&%2").arg(url.toString(), QString::number(QDateTime::currentMSecsSinceEpoch())));
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Origin", "https://launcher.finalfantasyxiv.com");
    request.setRawHeader("Referer",
                         QString("https://launcher.finalfantasyxiv.com/v600/index.html?rc_lang=%1&time=%2")
                             .arg("en-us", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH"))
                             .toUtf8());

    auto reply = mgr->get(request);
    co_await reply;

    auto document = QJsonDocument::fromJson(reply->readAll());

    auto headline = new Headline(this);
    if (document.isEmpty()) {
        headline->failedToLoad = true;
    }

    const auto parseNews = [](QJsonObject object) -> News {
        News news;
        news.date = QDateTime::fromString(object["date"].toString(), Qt::DateFormat::ISODate);
        news.id = object["id"].toString();
        news.tag = object["tag"].toString();
        news.title = object["title"].toString();

        if (object["url"].toString().isEmpty()) {
            news.url = QUrl(QString("https://na.finalfantasyxiv.com/lodestone/news/detail/%1").arg(news.id));
        } else {
            news.url = QUrl(object["url"].toString());
        }

        return news;
    };

    for (auto bannerObject : document.object()["banner"].toArray()) {
        auto banner = Banner();
        banner.link = QUrl(bannerObject.toObject()["link"].toString());
        banner.bannerImage = QUrl(bannerObject.toObject()["lsb_banner"].toString());

        headline->banners.push_back(banner);
    }

    for (auto newsObject : document.object()["news"].toArray()) {
        auto news = parseNews(newsObject.toObject());
        headline->news.push_back(news);
    }

    for (auto pinnedObject : document.object()["pinned"].toArray()) {
        auto pinned = parseNews(pinnedObject.toObject());
        headline->pinned.push_back(pinned);
    }

    for (auto pinnedObject : document.object()["topics"].toArray()) {
        auto pinned = parseNews(pinnedObject.toObject());
        headline->topics.push_back(pinned);
    }

    m_headline = headline;
    Q_EMIT newsChanged();
}

Headline *LauncherCore::headline()
{
    return m_headline;
}

bool LauncherCore::isSteam() const
{
    return m_isSteam;
}

void LauncherCore::openOfficialLauncher(Profile *profile)
{
    struct Argument {
        QString key, value;
    };

    QString executeArg("%1%2%3%4");
    QDateTime dateTime = QDateTime::currentDateTime();
    executeArg = executeArg.arg(dateTime.date().month() + 1, 2, 10, QLatin1Char('0'));
    executeArg = executeArg.arg(dateTime.date().day(), 2, 10, QLatin1Char('0'));
    executeArg = executeArg.arg(dateTime.time().hour(), 2, 10, QLatin1Char('0'));
    executeArg = executeArg.arg(dateTime.time().minute(), 2, 10, QLatin1Char('0'));

    QList<Argument> arguments{{"ExecuteArg", executeArg}, {"UserPath", Utility::toWindowsPath(profile->account()->getConfigDir().absolutePath())}};

    const QString argFormat = " /%1 =%2";

    QString argJoined;
    for (auto &arg : arguments) {
        argJoined += argFormat.arg(arg.key, arg.value.replace(" ", "  "));
    }

    QString finalArg = encryptGameArg(argJoined);

    auto launcherProcess = new QProcess(this);
    launcherProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(*profile, launcherProcess, {profile->gamePath() + "/boot/ffxivlauncher64.exe", finalArg}, false, true);
}

void LauncherCore::openSystemInfo(Profile *profile)
{
    auto sysinfoProcess = new QProcess(this);
    sysinfoProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(*profile, sysinfoProcess, {profile->gamePath() + "/boot/ffxivsysinfo64.exe"}, false, false);
}

void LauncherCore::openConfigBackup(Profile *profile)
{
    auto configProcess = new QProcess(this);
    configProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(*profile, configProcess, {profile->gamePath() + "/boot/ffxivconfig64.exe"}, false, false);
}

bool LauncherCore::isSteamDeck() const
{
    return steamApi->isDeck();
}

void LauncherCore::setIsSteam(bool isSteam)
{
    m_isSteam = isSteam;
}

Profile *LauncherCore::currentProfile() const
{
    return m_profileManager->getProfile(m_currentProfileIndex);
}

void LauncherCore::setCurrentProfile(Profile *profile)
{
    const int newIndex = m_profileManager->getProfileIndex(profile->name());
    if (newIndex != m_currentProfileIndex) {
        m_currentProfileIndex = newIndex;
        Q_EMIT currentProfileChanged();
    }
}