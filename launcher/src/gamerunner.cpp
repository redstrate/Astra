// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gamerunner.h"

#ifdef ENABLE_GAMEMODE
#include <gamemode_client.h>
#endif

#include "encryptedarg.h"
#include "launchercore.h"
#include "processlogger.h"
#include "utility.h"

using namespace Qt::StringLiterals;

GameRunner::GameRunner(LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
{
}

void GameRunner::beginGameExecutable(Profile &profile, const LoginAuth &auth)
{
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

    Q_EMIT m_launcher.successfulLaunch();
}

void GameRunner::beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const LoginAuth &auth)
{
    profile.setLoggedIn(true);

    auto gameProcess = new QProcess(this);
    gameProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    connect(gameProcess, &QProcess::finished, this, [this, &profile](int exitCode) {
        profile.setLoggedIn(false);
        Q_UNUSED(exitCode)
        Q_EMIT m_launcher.gameClosed();
    });

    auto args = getGameArgs(profile, auth);

    new ProcessLogger(gameProcess);

    launchExecutable(profile, gameProcess, {gameExecutablePath, args}, true, true);
}

void GameRunner::beginDalamudGame(const QString &gameExecutablePath, Profile &profile, const LoginAuth &auth)
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
    Utility::createPathIfNeeded(logDir);

    const QDir dalamudRuntimeDir = dalamudDir.absoluteFilePath(QStringLiteral("runtime"));
    const QDir dalamudAssetDir = dalamudDir.absoluteFilePath(QStringLiteral("assets"));
    const QDir dalamudConfigPath = userDalamudConfigDir.absoluteFilePath(QStringLiteral("dalamud-config.json"));

    const QDir dalamudInstallDir = dalamudDir.absoluteFilePath(profile.dalamudChannelName());
    const QString dalamudInjector = dalamudInstallDir.absoluteFilePath(QStringLiteral("Dalamud.Injector.exe"));

    auto dalamudProcess = new QProcess(this);
    connect(dalamudProcess, &QProcess::finished, this, [this, &profile](int exitCode) {
        profile.setLoggedIn(false);
        Q_UNUSED(exitCode)
        Q_EMIT m_launcher.gameClosed();
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

QString GameRunner::getGameArgs(const Profile &profile, const LoginAuth &auth)
{
    QList<std::pair<QString, QString>> gameArgs;
    gameArgs.push_back({QStringLiteral("DEV.DataPathType"), QString::number(1)});
    gameArgs.push_back({QStringLiteral("DEV.UseSqPack"), QString::number(1)});

    gameArgs.push_back({QStringLiteral("DEV.MaxEntitledExpansionID"), QString::number(auth.maxExpansion)});
    gameArgs.push_back({QStringLiteral("DEV.TestSID"), auth.SID});
    gameArgs.push_back({QStringLiteral("SYS.Region"), QString::number(auth.region)});
    gameArgs.push_back({QStringLiteral("language"), QString::number(profile.account()->language())});
    gameArgs.push_back({QStringLiteral("ver"), profile.baseGameVersion()});
    gameArgs.push_back({QStringLiteral("UserPath"), Utility::toWindowsPath(profile.account()->getConfigDir().absolutePath())});

    // FIXME: this should belong somewhere else...
    Utility::createPathIfNeeded(profile.account()->getConfigDir().absolutePath());
    Utility::createPathIfNeeded(profile.winePrefixPath());

    if (!auth.lobbyhost.isEmpty()) {
        gameArgs.push_back({QStringLiteral("DEV.GMServerHost"), auth.frontierHost});
        for (int i = 1; i < 9; i++) {
            gameArgs.push_back({QStringLiteral("DEV.LobbyHost0%1").arg(QString::number(i)), auth.lobbyhost});
            gameArgs.push_back({QStringLiteral("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
        }
    }

    if (profile.account()->license() == Account::GameLicense::WindowsSteam) {
        gameArgs.push_back({QStringLiteral("IsSteam"), QString::number(1)});
    }

    const QString argFormat = m_launcher.settings()->argumentsEncrypted() ? QStringLiteral(" /%1 =%2") : QStringLiteral(" %1=%2");

    QString argJoined;
    for (const auto &[key, value] : gameArgs) {
        argJoined += argFormat.arg(key, value);
    }

    return m_launcher.settings()->argumentsEncrypted() ? encryptGameArg(argJoined) : argJoined;
}

void GameRunner::launchExecutable(const Profile &profile, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup)
{
    QList<QString> arguments;
    auto env = process->processEnvironment();

    if (needsRegistrySetup) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        // FFXIV detects this as a "macOS" build by checking if Wine shows up
        const int value = profile.account()->license() == Account::GameLicense::macOS ? 0 : 1;
        addRegistryKey(profile, QStringLiteral("HKEY_CURRENT_USER\\Software\\Wine"), QStringLiteral("HideWineExports"), QString::number(value));

        // copy DXVK
        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir compatibilityToolDir = dataDir.absoluteFilePath(QStringLiteral("tool"));
        const QDir dxvkDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("dxvk"));
        const QDir dxvk64Dir = dxvkDir.absoluteFilePath(QStringLiteral("x64"));

        const QDir winePrefix = profile.winePrefixPath();
        const QDir driveC = winePrefix.absoluteFilePath(QStringLiteral("drive_c"));
        const QDir windows = driveC.absoluteFilePath(QStringLiteral("windows"));
        const QDir system32 = windows.absoluteFilePath(QStringLiteral("system32"));

        for (const auto &entry : dxvk64Dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
            if (QFile::exists(system32.absoluteFilePath(entry.fileName()))) {
                QFile::remove(system32.absoluteFilePath(entry.fileName()));
            }
            QFile::copy(entry.absoluteFilePath(), system32.absoluteFilePath(entry.fileName()));
        }
#endif
    }

    if (isGame && profile.gamescopeEnabled()) {
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

#ifdef ENABLE_GAMEMODE
    if (isGame && profile.gamemodeEnabled()) {
        gamemode_request_start();
    }
#endif

#if defined(Q_OS_LINUX)
    env.insert(QStringLiteral("WINEESYNC"), QString::number(1));
    env.insert(QStringLiteral("WINEFSYNC"), QString::number(1));
    env.insert(QStringLiteral("WINEFSYNC_FUTEX2"), QString::number(1));

    // env.insert(QStringLiteral("VK_LAYER_RENDERDOC_Capture"), QStringLiteral("VK_LAYER_RENDERDOC_Capture"));
    // env.insert(QStringLiteral("ENABLE_VULKAN_RENDERDOC_CAPTURE"), QString::number(1));

    const QString logDir = Utility::stateDirectory().absoluteFilePath(QStringLiteral("log"));

    env.insert(QStringLiteral("DXVK_LOG_PATH"), logDir);
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env.insert(QStringLiteral("WINEPREFIX"), profile.winePrefixPath());

    if (profile.wineType() == Profile::WineType::BuiltIn) {
        env.insert(QStringLiteral("WINEDLLOVERRIDES"), QStringLiteral("msquic=,mscoree=n,b;d3d9,d3d11,d3d10core,dxgi=n,b"));
    }

    arguments.push_back(profile.winePath());
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    if (isGame)
        process->setWorkingDirectory(profile.gamePath() + QStringLiteral("/game/"));

    process->setProcessEnvironment(env);

    process->start(executable, arguments);
}

void GameRunner::addRegistryKey(const Profile &settings, QString key, QString value, QString data)
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