// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <KLocalizedString>
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
#include "astra_log.h"
#include "compatibilitytoolinstaller.h"
#include "config.h"
#include "encryptedarg.h"
#include "launchercore.h"
#include "processlogger.h"
#include "sapphirelauncher.h"
#include "squarelauncher.h"
#include "utility.h"

void LauncherCore::setSSL(QNetworkRequest &request)
{
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherCore::setupIgnoreSSL(QNetworkReply *reply)
{
    Q_ASSERT(reply != nullptr);

    if (preferredProtocol() == QStringLiteral("http")) {
        connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &errors) {
            reply->ignoreSslErrors(errors);
        });
    }
}

void LauncherCore::buildRequest(const Profile &settings, QNetworkRequest &request)
{
    setSSL(request);

    if (settings.account()->license() == Account::GameLicense::macOS) {
        request.setHeader(QNetworkRequest::UserAgentHeader, QByteArrayLiteral("macSQEXAuthor/2.0.0(MacOSX; ja-jp)"));
    } else {
        request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString(QSysInfo::bootUniqueId())));
    }

    request.setRawHeader(QByteArrayLiteral("Accept"),
                         QByteArrayLiteral("image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, "
                                           "application/x-ms-xbap, */*"));
    request.setRawHeader(QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("gzip, deflate"));
    request.setRawHeader(QByteArrayLiteral("Accept-Language"), QByteArrayLiteral("en-us"));
}

void LauncherCore::launchGame(Profile &profile, const LoginAuth &auth)
{
    if (m_steamApi != nullptr) {
        m_steamApi->setLauncherMode(false);
    }

    beginGameExecutable(profile, auth);
}

QCoro::Task<> LauncherCore::beginLogin(LoginInformation &info)
{
    info.profile->account()->updateConfig();

    auto assetUpdater = new AssetUpdater(*info.profile, *this, this);
    if (co_await assetUpdater->update()) {
        if (info.profile->account()->isSapphire()) {
            m_sapphireLauncher->login(info.profile->account()->lobbyUrl(), info);
        } else {
            m_squareBoot->checkGateStatus(info);
        }
    }

    assetUpdater->deleteLater();
}

void LauncherCore::beginGameExecutable(Profile &profile, const LoginAuth &auth)
{
    Q_EMIT stageChanged(i18n("Launching game..."));

    QString gameExectuable;
    if (profile.directx9Enabled()) {
        gameExectuable = profile.gamePath() + QStringLiteral("/game/ffxiv.exe");
    } else {
        gameExectuable = profile.gamePath() + QStringLiteral("/game/ffxiv_dx11.exe");
    }

    if (profile.dalamudEnabled()) {
        beginDalamudGame(gameExectuable, profile, auth);
    } else {
        beginVanillaGame(gameExectuable, profile, auth);
    }

    Q_EMIT successfulLaunch();
}

void LauncherCore::beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const LoginAuth &auth)
{
    profile.setLoggedIn(true);

    auto gameProcess = new QProcess(this);
    gameProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    connect(gameProcess, &QProcess::finished, this, [this, &profile](int exitCode) {
        profile.setLoggedIn(false);
        Q_UNUSED(exitCode)
        Q_EMIT gameClosed();
    });

    auto args = getGameArgs(profile, auth);

    new ProcessLogger(gameProcess);

    launchExecutable(profile, gameProcess, {gameExecutablePath, args}, true, true);
}

void LauncherCore::beginDalamudGame(const QString &gameExecutablePath, Profile &profile, const LoginAuth &auth)
{
    profile.setLoggedIn(true);

    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QDir stateDir = Utility::stateDirectory();
    const QDir dalamudDir = dataDir.absoluteFilePath(QStringLiteral("dalamud"));

    const QDir dalamudConfigDir = configDir.absoluteFilePath(QStringLiteral("dalamud"));
    const QDir userDalamudConfigDir = dalamudConfigDir.absoluteFilePath(profile.account()->uuid());

    const QDir dalamudBasePluginDir = dalamudDir.absoluteFilePath(QStringLiteral("plugins"));
    const QDir dalamudUserPluginDir = dalamudBasePluginDir.absoluteFilePath(profile.account()->uuid());

    // Some really dumb plugins check for "installedPlugins" in their assembly directory FOR SOME REASON,
    // so we need to match typical XIVQuickLauncher behavior here. Why? I have no clue.
    const QDir dalamudPluginDir = dalamudUserPluginDir.absoluteFilePath(QStringLiteral("installedPlugins"));

    const QString logDir = stateDir.absoluteFilePath(QStringLiteral("log"));

    if (!QDir().exists(logDir))
        QDir().mkpath(logDir);

    const QDir dalamudRuntimeDir = dalamudDir.absoluteFilePath(QStringLiteral("runtime"));
    const QDir dalamudAssetDir = dalamudDir.absoluteFilePath(QStringLiteral("assets"));
    const QDir dalamudConfigPath = userDalamudConfigDir.absoluteFilePath(QStringLiteral("dalamud-config.json"));

    const QDir dalamudInstallDir = dalamudDir.absoluteFilePath(profile.dalamudChannelName());
    const QString dalamudInjector = dalamudInstallDir.absoluteFilePath(QStringLiteral("Dalamud.Injector.exe"));

    auto dalamudProcess = new QProcess(this);
    connect(dalamudProcess, &QProcess::finished, this, [this, &profile](int exitCode) {
        profile.setLoggedIn(false);
        Q_UNUSED(exitCode)
        Q_EMIT gameClosed();
    });

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("DALAMUD_RUNTIME"), Utility::toWindowsPath(dalamudRuntimeDir));

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    env.insert(QStringLiteral("XL_WINEONLINUX"), QStringLiteral("true"));
#endif
    dalamudProcess->setProcessEnvironment(env);

    new ProcessLogger(dalamudProcess);

    const auto args = getGameArgs(profile, auth);

    launchExecutable(profile,
                     dalamudProcess,
                     {Utility::toWindowsPath(dalamudInjector),
                      QStringLiteral("launch"),
                      QStringLiteral("-m"),
                      profile.dalamudInjectMethod() == Profile::DalamudInjectMethod::Entrypoint ? QStringLiteral("entrypoint") : QStringLiteral("inject"),
                      QStringLiteral("--game=") + Utility::toWindowsPath(gameExecutablePath),
                      QStringLiteral("--dalamud-configuration-path=") + Utility::toWindowsPath(dalamudConfigPath),
                      QStringLiteral("--dalamud-plugin-directory=") + Utility::toWindowsPath(dalamudPluginDir),
                      QStringLiteral("--dalamud-asset-directory=") + Utility::toWindowsPath(dalamudAssetDir),
                      QStringLiteral("--dalamud-client-language=") + QString::number(profile.account()->language()),
                      QStringLiteral("--dalamud-delay-initialize=") + QString::number(profile.dalamudInjectDelay()),
                      QStringLiteral("--logpath=") + Utility::toWindowsPath(logDir),
                      QStringLiteral("--"),
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
    gameArgs.push_back({QStringLiteral("DEV.DataPathType"), QString::number(1)});
    gameArgs.push_back({QStringLiteral("DEV.UseSqPack"), QString::number(1)});

    gameArgs.push_back({QStringLiteral("DEV.MaxEntitledExpansionID"), QString::number(auth.maxExpansion)});
    gameArgs.push_back({QStringLiteral("DEV.TestSID"), auth.SID});
    gameArgs.push_back({QStringLiteral("SYS.Region"), QString::number(auth.region)});
    gameArgs.push_back({QStringLiteral("language"), QString::number(profile.account()->language())});
    gameArgs.push_back({QStringLiteral("ver"), profile.baseGameVersion()});
    gameArgs.push_back({QStringLiteral("UserPath"), Utility::toWindowsPath(profile.account()->getConfigDir().absolutePath())});

    // FIXME: this should belong somewhere else...
    if (!QDir().exists(profile.account()->getConfigDir().absolutePath())) {
        QDir().mkpath(profile.account()->getConfigDir().absolutePath());
    }

    if (!QDir().exists(profile.winePrefixPath())) {
        QDir().mkpath(profile.winePrefixPath());
    }

    if (!auth.lobbyhost.isEmpty()) {
        gameArgs.push_back({QStringLiteral("DEV.GMServerHost"), auth.frontierHost});
        for (int i = 1; i < 9; i++) {
            gameArgs.push_back({QStringLiteral("DEV.LobbyHost0%1").arg(QString::number(i)), auth.lobbyhost});
            gameArgs.push_back({QStringLiteral("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
        }
    }

    if (profile.account()->license() == Account::GameLicense::WindowsSteam) {
        gameArgs.push_back({QStringLiteral("IsSteam"), QStringLiteral("1")});
    }

    const QString argFormat = argumentsEncrypted() ? QStringLiteral(" /%1 =%2") : QStringLiteral(" %1=%2");

    QString argJoined;
    for (const auto &arg : gameArgs) {
        argJoined += argFormat.arg(arg.key, arg.value);
    }

    return argumentsEncrypted() ? encryptGameArg(argJoined) : argJoined;
}

void LauncherCore::launchExecutable(const Profile &profile, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup)
{
    QList<QString> arguments;
    auto env = process->processEnvironment();

    if (needsRegistrySetup) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        if (profile.account()->license() == Account::GameLicense::macOS) {
            addRegistryKey(profile, QStringLiteral("HKEY_CURRENT_USER\\Software\\Wine"), QStringLiteral("HideWineExports"), QStringLiteral("0"));
        } else {
            addRegistryKey(profile, QStringLiteral("HKEY_CURRENT_USER\\Software\\Wine"), QStringLiteral("HideWineExports"), QStringLiteral("1"));
        }
#endif
    }

#if defined(Q_OS_LINUX)
    if (isGame) {
        if (profile.gamescopeEnabled()) {
            arguments.push_back(QStringLiteral("gamescope"));

            if (profile.gamescopeFullscreen())
                arguments.push_back(QStringLiteral("-f"));

            if (profile.gamescopeBorderless())
                arguments.push_back(QStringLiteral("-b"));

            if (profile.gamescopeWidth() > 0)
                arguments.push_back(QStringLiteral("-w ") + QString::number(profile.gamescopeWidth()));

            if (profile.gamescopeHeight() > 0)
                arguments.push_back(QStringLiteral("-h ") + QString::number(profile.gamescopeHeight()));

            if (profile.gamescopeRefreshRate() > 0)
                arguments.push_back(QStringLiteral("-r ") + QString::number(profile.gamescopeRefreshRate()));
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
        env.insert(QStringLiteral("WINEESYNC"), QString::number(1));
        env.insert(QStringLiteral("WINEFSYNC"), QString::number(1));
        env.insert(QStringLiteral("WINEFSYNC_FUTEX2"), QString::number(1));
    }

    const QString logDir = Utility::stateDirectory().absoluteFilePath(QStringLiteral("log"));

    env.insert(QStringLiteral("DXVK_LOG_PATH"), logDir);
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    if (m_isSteam) {
        const QDir steamDirectory = QProcessEnvironment::systemEnvironment().value(QStringLiteral("STEAM_COMPAT_CLIENT_INSTALL_PATH"));
        const QDir compatData =
            QProcessEnvironment::systemEnvironment().value(QStringLiteral("STEAM_COMPAT_DATA_PATH")); // TODO: do these have to exist on the root steam folder?

        const QString steamAppsPath = steamDirectory.absoluteFilePath(QStringLiteral("steamapps/common"));

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
            if (dirName.contains(QLatin1String("Proton"))) {
                if (dirName == QLatin1String("Proton - Experimental")) {
                    highestVersion.setPath(dir);
                    break;
                } else {
                    QString version = dirName.remove(QLatin1String("Proton "));
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

        env.insert(QStringLiteral("STEAM_COMPAT_CLIENT_INSTALL_PATH"), steamDirectory.absolutePath());
        env.insert(QStringLiteral("STEAM_COMPAT_DATA_PATH"), compatData.absolutePath());

        arguments.push_back(highestVersion.absoluteFilePath(QStringLiteral("proton")));
        arguments.push_back(QStringLiteral("run"));
    } else {
        env.insert(QStringLiteral("WINEPREFIX"), profile.winePrefixPath());

        // XIV on Mac bundle their own Wine install directory, complete with libs etc
        if (profile.wineType() == Profile::WineType::XIVOnMac) {
            // TODO: don't hardcode this
            QString xivLibPath = QStringLiteral(
                "/Applications/XIV on Mac.app/Contents/Resources/wine/lib:/Applications/XIV on "
                "Mac.app/Contents/Resources/MoltenVK/modern");

            env.insert(QStringLiteral("DYLD_FALLBACK_LIBRARY_PATH"), xivLibPath);
            env.insert(QStringLiteral("DYLD_VERSIONED_LIBRARY_PATH"), xivLibPath);
            env.insert(QStringLiteral("MVK_CONFIG_FULL_IMAGE_VIEW_SWIZZLE"), QStringLiteral("1"));
            env.insert(QStringLiteral("MVK_CONFIG_RESUME_LOST_DEVICE"), QStringLiteral("1"));
            env.insert(QStringLiteral("MVK_ALLOW_METAL_FENCES"), QStringLiteral("1"));
            env.insert(QStringLiteral("MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS"), QStringLiteral("1"));
        }

#if defined(FLATPAK)
        arguments.push_back(QStringLiteral("flatpak-spawn"));
        arguments.push_back(QStringLiteral("--host"));
#endif
        arguments.push_back(profile.winePath());
    }
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    if (isGame)
        process->setWorkingDirectory(profile.gamePath() + QStringLiteral("/game/"));

    process->setProcessEnvironment(env);

    process->start(executable, arguments);
}

void LauncherCore::readInitialInformation()
{
    m_profileManager->load();
    m_accountManager->load();

    // restore profile -> account connections
    for (auto profile : m_profileManager->profiles()) {
        if (auto account = m_accountManager->getByUuid(profile->accountUuid())) {
            profile->setAccount(account);
        }
    }

    // set default profile, if found
    if (auto profile = m_profileManager->getProfileByUUID(m_config->currentProfile()); profile != nullptr) {
        setCurrentProfile(profile);
    }

    m_loadingFinished = true;
    Q_EMIT loadingFinished();
}

LauncherCore::LauncherCore()
{
    m_config = new Config(KSharedConfig::openConfig("astrarc", KConfig::SimpleConfig, QStandardPaths::AppConfigLocation));
    mgr = new QNetworkAccessManager(this);
    m_sapphireLauncher = new SapphireLauncher(*this, this);
    m_squareLauncher = new SquareLauncher(*this, this);
    m_squareBoot = new SquareBoot(*this, *m_squareLauncher, this);
    m_profileManager = new ProfileManager(*this, this);
    m_accountManager = new AccountManager(*this, this);

    readInitialInformation();
}

bool LauncherCore::checkIfInPath(const QString &program)
{
    const auto pathList = qgetenv("PATH").split(':');

    return std::any_of(pathList.cbegin(), pathList.cend(), [program](const QByteArray &path) {
        QFileInfo fileInfo(path + QStringLiteral("/") + program);
        return fileInfo.exists() && fileInfo.isFile();
    });
}

void LauncherCore::addRegistryKey(const Profile &settings, QString key, QString value, QString data)
{
    auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(settings,
                     process,
                     {QStringLiteral("reg"),
                      QStringLiteral("add"),
                      std::move(key),
                      QStringLiteral("/v"),
                      std::move(value),
                      QStringLiteral("/d"),
                      std::move(data),
                      QStringLiteral("/f")},
                     false,
                     false);
}

void LauncherCore::login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword)
{
    Q_ASSERT(profile != nullptr);

    auto loginInformation = new LoginInformation(this);
    loginInformation->profile = profile;
    loginInformation->username = username;
    loginInformation->password = password;
    loginInformation->oneTimePassword = oneTimePassword;

    if (profile->account()->rememberPassword()) {
        profile->account()->setPassword(password);
    }

    beginLogin(*loginInformation);
}

bool LauncherCore::autoLogin(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    QString otp;
    if (profile->account()->useOTP()) {
        if (!profile->account()->rememberOTP()) {
            Q_EMIT loginError(i18n("This account does not have an OTP secret set, but requires it for login."));
            return false;
        }

        otp = profile->account()->getOTP();
        if (otp.isEmpty()) {
            Q_EMIT loginError(i18n("Failed to generate OTP, review the stored secret."));
            return false;
        }
    }

    login(profile, profile->account()->name(), profile->account()->getPassword(), otp);
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
    return m_config->closeWhenLaunched();
}

void LauncherCore::setCloseWhenLaunched(const bool value)
{
    if (value != m_config->closeWhenLaunched()) {
        m_config->setCloseWhenLaunched(value);
        m_config->save();
        Q_EMIT closeWhenLaunchedChanged();
    }
}

bool LauncherCore::showNews() const
{
    return m_config->showNews();
}

void LauncherCore::setShowNews(const bool value)
{
    if (value != m_config->showNews()) {
        m_config->setShowNews(value);
        m_config->save();
        Q_EMIT showNewsChanged();
    }
}

bool LauncherCore::showDevTools() const
{
    return m_config->showDevTools();
}

void LauncherCore::setShowDevTools(const bool value)
{
    if (value != m_config->showDevTools()) {
        m_config->setShowDevTools(value);
        m_config->save();
        Q_EMIT showDevToolsChanged();
    }
}

bool LauncherCore::keepPatches() const
{
    return m_config->keepPatches();
}

void LauncherCore::setKeepPatches(const bool value)
{
    if (value != m_config->keepPatches()) {
        m_config->setKeepPatches(value);
        m_config->save();
        Q_EMIT keepPatchesChanged();
    }
}

QString LauncherCore::dalamudDistribServer() const
{
    return m_config->dalamudDistribServer();
}

void LauncherCore::setDalamudDistribServer(const QString &value)
{
    if (value != m_config->dalamudDistribServer()) {
        m_config->setDalamudDistribServer(value);
        m_config->save();
        Q_EMIT dalamudDistribServerChanged();
    }
}

QString LauncherCore::squareEnixServer() const
{
    return m_config->squareEnixServer();
}

void LauncherCore::setSquareEnixServer(const QString &value)
{
    if (value != m_config->squareEnixServer()) {
        m_config->setSquareEnixServer(value);
        m_config->save();
        Q_EMIT squareEnixServerChanged();
    }
}

QString LauncherCore::squareEnixLoginServer() const
{
    return m_config->squareEnixLoginServer();
}

void LauncherCore::setSquareEnixLoginServer(const QString &value)
{
    if (value != m_config->squareEnixLoginServer()) {
        m_config->setSquareEnixLoginServer(value);
        m_config->save();
        Q_EMIT squareEnixLoginServerChanged();
    }
}

QString LauncherCore::xivApiServer() const
{
    return m_config->xivApiServer();
}

void LauncherCore::setXivApiServer(const QString &value)
{
    if (value != m_config->xivApiServer()) {
        m_config->setXivApiServer(value);
        m_config->save();
        Q_EMIT xivApiServerChanged();
    }
}

QString LauncherCore::preferredProtocol() const
{
    return m_config->preferredProtocol();
}

void LauncherCore::setPreferredProtocol(const QString &value)
{
    if (value != m_config->preferredProtocol()) {
        m_config->setPreferredProtocol(value);
        m_config->save();
        Q_EMIT preferredProtocolChanged();
    }
}

QString LauncherCore::screenshotDir() const
{
    return m_config->screenshotDir();
}

void LauncherCore::setScreenshotDir(const QString &value)
{
    if (value != m_config->screenshotDir()) {
        m_config->setScreenshotDir(value);
        m_config->save();
        Q_EMIT screenshotDirChanged();
    }
}

bool LauncherCore::argumentsEncrypted() const
{
    return m_config->encryptArguments();
}

void LauncherCore::setArgumentsEncrypted(const bool value)
{
    if (m_config->encryptArguments() != value) {
        m_config->setEncryptArguments(value);
        m_config->save();
        Q_EMIT encryptedArgumentsChanged();
    }
}

[[nodiscard]] QString LauncherCore::autoLoginProfileName() const
{
    return m_config->autoLoginProfile();
}

[[nodiscard]] Profile *LauncherCore::autoLoginProfile() const
{
    if (m_config->autoLoginProfile().isEmpty()) {
        return nullptr;
    }
    return m_profileManager->getProfileByUUID(m_config->autoLoginProfile());
}

void LauncherCore::setAutoLoginProfile(Profile *profile)
{
    if (profile == nullptr) {
        m_config->setAutoLoginProfile({});
        m_config->save();
        Q_EMIT autoLoginProfileChanged();
        return;
    }

    auto uuid = profile->uuid();
    if (uuid != m_config->autoLoginProfile()) {
        m_config->setAutoLoginProfile(uuid);
        m_config->save();
        Q_EMIT autoLoginProfileChanged();
    }
}

void LauncherCore::refreshNews()
{
    fetchNews();
}

QCoro::Task<> LauncherCore::fetchNews()
{
    qInfo(ASTRA_LOG) << "Fetching news...";

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("lang"), QStringLiteral("en-us"));
    query.addQueryItem(QStringLiteral("media"), QStringLiteral("pcapp"));

    QUrl url;
    url.setScheme(preferredProtocol());
    url.setHost(QStringLiteral("frontier.%1").arg(squareEnixServer()));
    url.setPath(QStringLiteral("/news/headline.json"));
    url.setQuery(query);

    QNetworkRequest request(QStringLiteral("%1&%2").arg(url.toString(), QString::number(QDateTime::currentMSecsSinceEpoch())));
    request.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, text/plain, */*"));
    request.setRawHeader(QByteArrayLiteral("Origin"), QByteArrayLiteral("https://launcher.finalfantasyxiv.com"));
    request.setRawHeader(QByteArrayLiteral("Referer"),
                         QStringLiteral("https://launcher.finalfantasyxiv.com/v600/index.html?rc_lang=%1&time=%2")
                             .arg("en-us", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH"))
                             .toUtf8());
    Utility::printRequest(QStringLiteral("GET"), request);

    auto reply = mgr->get(request);
    co_await reply;

    auto document = QJsonDocument::fromJson(reply->readAll());

    auto headline = new Headline(this);
    if (document.isEmpty()) {
        headline->failedToLoad = true;
    }

    const auto parseNews = [](QJsonObject object) -> News {
        News news;
        news.date = QDateTime::fromString(object[QLatin1String("date")].toString(), Qt::DateFormat::ISODate);
        news.id = object[QLatin1String("id")].toString();
        news.tag = object[QLatin1String("tag")].toString();
        news.title = object[QLatin1String("title")].toString();

        if (object[QLatin1String("url")].toString().isEmpty()) {
            news.url = QUrl(QStringLiteral("https://na.finalfantasyxiv.com/lodestone/news/detail/%1").arg(news.id));
        } else {
            news.url = QUrl(object[QLatin1String("url")].toString());
        }

        return news;
    };

    for (auto bannerObject : document.object()[QLatin1String("banner")].toArray()) {
        auto banner = Banner();
        banner.link = QUrl(bannerObject.toObject()[QLatin1String("link")].toString());
        banner.bannerImage = QUrl(bannerObject.toObject()[QLatin1String("lsb_banner")].toString());

        headline->banners.push_back(banner);
    }

    for (auto newsObject : document.object()[QLatin1String("news")].toArray()) {
        auto news = parseNews(newsObject.toObject());
        headline->news.push_back(news);
    }

    for (auto pinnedObject : document.object()[QLatin1String("pinned")].toArray()) {
        auto pinned = parseNews(pinnedObject.toObject());
        headline->pinned.push_back(pinned);
    }

    for (auto pinnedObject : document.object()[QLatin1String("topics")].toArray()) {
        auto pinned = parseNews(pinnedObject.toObject());
        headline->topics.push_back(pinned);
    }

    m_headline = headline;
    Q_EMIT newsChanged();
}

Headline *LauncherCore::headline() const
{
    return m_headline;
}

bool LauncherCore::isSteam() const
{
    return m_isSteam;
}

bool LauncherCore::isSteamDeck() const
{
    if (m_steamApi != nullptr) {
        return m_steamApi->isDeck();
    } else {
        return false;
    }
}

Profile *LauncherCore::currentProfile() const
{
    return m_profileManager->getProfile(m_currentProfileIndex);
}

void LauncherCore::setCurrentProfile(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    const int newIndex = m_profileManager->getProfileIndex(profile->uuid());
    if (newIndex != m_currentProfileIndex) {
        m_currentProfileIndex = newIndex;
        m_config->setCurrentProfile(profile->uuid());
        m_config->save();
        Q_EMIT currentProfileChanged();
    }
}

void LauncherCore::clearAvatarCache()
{
    const auto cacheLocation = QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + QStringLiteral("/avatars");
    if (QDir(cacheLocation).exists()) {
        QDir(cacheLocation).removeRecursively();
    }
}

void LauncherCore::initializeSteam()
{
    m_isSteam = true;
    m_steamApi = new SteamAPI(*this, this);
    m_steamApi->setLauncherMode(true);
}
