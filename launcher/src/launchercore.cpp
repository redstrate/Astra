#include "gameinstaller.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QStandardPaths>
#include <algorithm>
#include <utility>

#ifdef ENABLE_GAMEMODE
#include <gamemode_client.h>
#endif

#include "account.h"
#include "assetupdater.h"
#include "config.h"
#include "encryptedarg.h"
#include "launchercore.h"
#include "sapphirelauncher.h"
#include "squarelauncher.h"

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
    connect(gameProcess, qOverload<int>(&QProcess::finished), this, [this](int exitCode) {
        Q_UNUSED(exitCode)
        Q_EMIT gameClosed();
    });

    auto args = getGameArgs(profile, auth);

    launchExecutable(profile, gameProcess, {gameExecutablePath, args}, true, true);
}

void LauncherCore::beginDalamudGame(const QString &gameExecutablePath, const Profile &profile, const LoginAuth &auth)
{
    QString gamePath = gameExecutablePath;
    gamePath = "Z:" + gamePath.replace('/', '\\');

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dataDir = "Z:" + dataDir.replace('/', '\\');

    auto dalamudProcess = new QProcess(this);
    connect(dalamudProcess, qOverload<int>(&QProcess::finished), this, [this](int exitCode) {
        Q_UNUSED(exitCode)
        Q_EMIT gameClosed();
    });

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DALAMUD_RUNTIME", dataDir + "\\DalamudRuntime");

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    env.insert("XL_WINEONLINUX", "true");
#endif
    dalamudProcess->setProcessEnvironment(env);

    auto args = getGameArgs(profile, auth);

    launchExecutable(profile,
                     dalamudProcess,
                     {dataDir + "/Dalamud/" + "Dalamud.Injector.exe",
                      "launch",
                      "-m",
                      "inject",
                      "--game=" + gamePath,
                      "--dalamud-configuration-path=" + dataDir + "\\dalamudConfig.json",
                      "--dalamud-plugin-directory=" + dataDir + "\\installedPlugins",
                      "--dalamud-asset-directory=" + dataDir + "\\DalamudAssets",
                      "--dalamud-client-language=" + QString::number(profile.language()),
                      "--logpath=" + dataDir,
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
    gameArgs.push_back({"language", QString::number(profile.language())});
    gameArgs.push_back({"ver", profile.repositories.repositories[0].version});

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
        const QString steamDirectory = QProcessEnvironment::systemEnvironment().value("STEAM_COMPAT_CLIENT_INSTALL_PATH");
        const QString compatData =
            QProcessEnvironment::systemEnvironment().value("STEAM_COMPAT_DATA_PATH"); // TODO: do these have to exist on the root steam folder?
        const QString protonPath = steamDirectory + "/steamapps/common/Proton 7.0";

        env.insert("STEAM_COMPAT_CLIENT_INSTALL_PATH", steamDirectory);
        env.insert("STEAM_COMPAT_DATA_PATH", compatData);

        arguments.push_back(protonPath + "/proton");
        arguments.push_back("run");
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

LauncherCore::LauncherCore(bool isSteam)
    : m_isSteam(isSteam)
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
    assetUpdater->update();

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
}

bool LauncherCore::autoLogin(Profile &profile)
{
    // TODO: when login fails, we need some way to propagate this back? or not?
    login(&profile, profile.account()->name(), profile.account()->getPassword(), profile.account()->getOTP());

    return true;
}

GameInstaller *LauncherCore::createInstaller(Profile &profile)
{
    return new GameInstaller(*this, profile, this);
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

void LauncherCore::refreshNews()
{
    QUrlQuery query;
    query.addQueryItem("lang", "en-us");
    query.addQueryItem("media", "pcapp");

    QUrl url;
    url.setScheme("https");
    url.setHost("frontier.ffxiv.com");
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
    QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
        auto document = QJsonDocument::fromJson(reply->readAll());

        auto headline = new Headline(this);

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
    });
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

    QList<Argument> arguments;
    arguments.push_back({"ExecuteArg", executeArg});

    // find user path
    QString userPath;

    // TODO: don't put this here
    QString searchDir;
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    searchDir = profile->winePrefixPath() + "/drive_c/users";
#else
    searchDir = "C:/Users";
#endif

    QDirIterator it(searchDir);
    while (it.hasNext()) {
        QString dir = it.next();
        QFileInfo fi(dir);
        QString fileName = fi.fileName();

        // FIXME: is there no easier way to filter out these in Qt?
        if (fi.fileName() != "Public" && fi.fileName() != "." && fi.fileName() != "..") {
            userPath = fileName;
        }
    }

    arguments.push_back({"UserPath", QString(R"(C:\Users\%1\Documents\My Games\FINAL FANTASY XIV - A Realm Reborn)").arg(userPath)});

    const QString argFormat = " /%1 =%2";

    QString argJoined;
    for (auto &arg : arguments) {
        argJoined += argFormat.arg(arg.key, arg.value.replace(" ", "  "));
    }

    QString finalArg = encryptGameArg(argJoined);

    auto launcherProcess = new QProcess(this);
    launchExecutable(*profile, launcherProcess, {profile->gamePath() + "/boot/ffxivlauncher64.exe", finalArg}, false, true);
}

void LauncherCore::openSystemInfo(Profile *profile)
{
    auto sysinfoProcess = new QProcess(this);
    launchExecutable(*profile, sysinfoProcess, {profile->gamePath() + "/boot/ffxivsysinfo64.exe"}, false, false);
}

void LauncherCore::openConfigBackup(Profile *profile)
{
    auto configProcess = new QProcess(this);
    launchExecutable(*profile, configProcess, {profile->gamePath() + "/boot/ffxivconfig64.exe"}, false, false);
}

bool LauncherCore::isSteamDeck() const
{
    return steamApi->isDeck();
}
