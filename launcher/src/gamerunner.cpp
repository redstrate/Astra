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

void GameRunner::beginGameExecutable(Profile &profile, const std::optional<LoginAuth> &auth)
{
    QString gameExectuable;
    if (profile.directx9Enabled() && profile.hasDirectx9()) {
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

void GameRunner::beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const std::optional<LoginAuth> &auth)
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

void GameRunner::beginDalamudGame(const QString &gameExecutablePath, Profile &profile, const std::optional<LoginAuth> &auth)
{
    profile.setLoggedIn(true);

    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QDir dalamudDir = dataDir.absoluteFilePath(QStringLiteral("dalamud"));

    const QDir dalamudConfigDir = configDir.absoluteFilePath(QStringLiteral("dalamud"));
    const QDir userDalamudConfigDir = dalamudConfigDir.absoluteFilePath(profile.account()->uuid());

    const QDir dalamudBasePluginDir = dalamudDir.absoluteFilePath(QStringLiteral("plugins"));
    const QDir dalamudUserPluginDir = dalamudBasePluginDir.absoluteFilePath(profile.account()->uuid());

    // Some really dumb plugins check for "installedPlugins" in their assembly directory FOR SOME REASON,
    // so we need to match typical XIVQuickLauncher behavior here. Why? I have no clue.
    const QDir dalamudPluginDir = dalamudUserPluginDir.absoluteFilePath(QStringLiteral("installedPlugins"));

    const QString logDir = dataDir.absoluteFilePath(QStringLiteral("log"));
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

QString GameRunner::getGameArgs(const Profile &profile, const std::optional<LoginAuth> &auth)
{
    QList<std::pair<QString, QString>> gameArgs;

    if (!profile.isBenchmark()) {
        gameArgs.push_back({QStringLiteral("DEV.DataPathType"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("DEV.UseSqPack"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("ver"), profile.baseGameVersion()});
        gameArgs.push_back({QStringLiteral("resetconfig"), QStringLiteral("0")});
    }

    if (auth) {
        gameArgs.push_back({QStringLiteral("DEV.MaxEntitledExpansionID"), QString::number(auth->maxExpansion)});
        gameArgs.push_back({QStringLiteral("DEV.TestSID"), auth->SID});
        gameArgs.push_back({QStringLiteral("SYS.Region"), QString::number(auth->region)});
        gameArgs.push_back({QStringLiteral("language"), QString::number(profile.account()->language())});
        gameArgs.push_back({QStringLiteral("UserPath"), Utility::toWindowsPath(profile.account()->getConfigDir().absolutePath())});

        Utility::createPathIfNeeded(profile.account()->getConfigDir().absolutePath());
    } else if (profile.isBenchmark()) {
        gameArgs.push_back({QStringLiteral("SYS.Language"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.Fps"), QString::number(0)});
        gameArgs.push_back({QStringLiteral("SYS.WaterWet_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.OcclusionCulling_DX11"), QString::number(0)});
        gameArgs.push_back({QStringLiteral("SYS.LodType_DX11"), QString::number(0)});
        gameArgs.push_back({QStringLiteral("SYS.ReflectionType_DX11"), QString::number(3)});
        gameArgs.push_back({QStringLiteral("SYS.AntiAliasing_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.TranslucentQuality_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.GrassQuality_DX11"), QString::number(3)});
        gameArgs.push_back({QStringLiteral("SYS.ShadowLOD_DX11"), QString::number(0)});
        gameArgs.push_back({QStringLiteral("SYS.ShadowVisibilityTypeSelf_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.ShadowVisibilityTypeOther_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.ShadowTextureSizeType_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.ShadowCascadeCountType_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.ShadowSoftShadowType_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.PhysicsTypeSelf_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.PhysicsTypeOther_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.TextureFilterQuality_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.TextureAnisotropicQuality_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.Vignetting_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.RadialBlur_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.SSAO_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.Glare_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.DepthOfField_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.ParallaxOcclusion_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.Tessellation_DX11"), QString::number(0)});
        gameArgs.push_back({QStringLiteral("SYS.GlareRepresentation_DX11"), QString::number(1)});
        gameArgs.push_back({QStringLiteral("SYS.DistortionWater_DX11"), QString::number(2)});
        gameArgs.push_back({QStringLiteral("SYS.ScreenMode"), QString::number(0)});
        gameArgs.push_back({QStringLiteral("SYS.ScreenWidth"), QString::number(1920)});
        gameArgs.push_back({QStringLiteral("SYS.ScreenHeight"), QString::number(1080)});
    }

    // FIXME: this should belong somewhere else...
    if (LauncherCore::needsCompatibilityTool())
        Utility::createPathIfNeeded(profile.winePrefixPath());

    if (auth) {
        if (!auth->lobbyhost.isEmpty()) {
            gameArgs.push_back({QStringLiteral("DEV.GMServerHost"), auth->frontierHost});
            for (int i = 1; i < 9; i++) {
                gameArgs.push_back({QStringLiteral("DEV.LobbyHost0%1").arg(QString::number(i)), auth->lobbyhost});
                gameArgs.push_back({QStringLiteral("DEV.LobbyPort0%1").arg(QString::number(i)), QString::number(54994)});
            }
        }

        if (profile.account()->license() == Account::GameLicense::WindowsSteam) {
            gameArgs.push_back({QStringLiteral("IsSteam"), QString::number(1)});
        }
    }

    const QString argFormat = !profile.isBenchmark() && m_launcher.settings()->argumentsEncrypted() ? QStringLiteral(" /%1 =%2") : QStringLiteral(" %1=%2");

    QString argJoined;
    for (const auto &[key, value] : gameArgs) {
        argJoined += argFormat.arg(key, value);
    }

    return !profile.isBenchmark() && m_launcher.settings()->argumentsEncrypted() ? encryptGameArg(argJoined) : argJoined;
}

void GameRunner::launchExecutable(const Profile &profile, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup)
{
    QList<QString> arguments;
    auto env = process->processEnvironment();

    if (needsRegistrySetup) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        // FFXIV detects this as a "macOS" build by checking if Wine shows up
        if (!profile.isBenchmark()) {
            const int value = profile.account()->license() == Account::GameLicense::macOS ? 0 : 1;
            addRegistryKey(profile, QStringLiteral("HKEY_CURRENT_USER\\Software\\Wine"), QStringLiteral("HideWineExports"), QString::number(value));
        }

        setWindowsVersion(profile, QStringLiteral("win7"));

        // TODO: band-aid fix to wait for wine to exit, otherwise gamescope will collide
        QThread::sleep(2);

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

        if (profile.gamescopeWidth() > 0) {
            arguments.push_back(QStringLiteral("-w"));
            arguments.push_back(QString::number(profile.gamescopeWidth()));
        }

        if (profile.gamescopeHeight() > 0) {
            arguments.push_back(QStringLiteral("-h"));
            arguments.push_back(QString::number(profile.gamescopeHeight()));
        }

        if (profile.gamescopeRefreshRate() > 0) {
            arguments.push_back(QStringLiteral("-r"));
            arguments.push_back(QString::number(profile.gamescopeRefreshRate()));
        }

        arguments.push_back(QStringLiteral("--"));
    }

#ifdef ENABLE_GAMEMODE
    if (isGame && profile.gamemodeEnabled()) {
        gamemode_request_start();
    }
#endif

    if (m_launcher.settings()->enableRenderDocCapture()) {
        env.insert(QStringLiteral("VK_LAYER_RENDERDOC_Capture"), QStringLiteral("VK_LAYER_RENDERDOC_Capture"));
        env.insert(QStringLiteral("ENABLE_VULKAN_RENDERDOC_CAPTURE"), QString::number(1));
    }

#if defined(Q_OS_LINUX)
    env.insert(QStringLiteral("WINEESYNC"), QString::number(1));
    env.insert(QStringLiteral("WINEFSYNC"), QString::number(1));
    env.insert(QStringLiteral("WINEFSYNC_FUTEX2"), QString::number(1));

    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString logDir = dataDir.absoluteFilePath(QStringLiteral("log"));

    env.insert(QStringLiteral("DXVK_LOG_PATH"), logDir);
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env.insert(QStringLiteral("WINEPREFIX"), profile.winePrefixPath());

    if (profile.wineType() == Profile::WineType::BuiltIn) {
        env.insert(QStringLiteral("WINEDLLOVERRIDES"), QStringLiteral("msquic=,mscoree=n,b;d3d9,d3d11,d3d10core,dxgi=n,b"));
    }

    arguments.push_back(profile.winePath());
#endif

    if (!profile.isBenchmark() && profile.account()->license() == Account::GameLicense::WindowsSteam) {
        env.insert(QStringLiteral("IS_FFXIV_LAUNCH_FROM_STEAM"), QStringLiteral("1"));
    }

    arguments.append(args);

    const QString executable = arguments.takeFirst();

    if (isGame) {
        if (profile.isBenchmark()) {
            // Benchmarks usually have some data located in the root
            process->setWorkingDirectory(profile.gamePath());
        } else {
            process->setWorkingDirectory(profile.gamePath() + QStringLiteral("/game/"));
        }
    }

    process->setProcessEnvironment(env);

    process->setProgram(executable);
    process->setArguments(arguments);

    process->start();
}

void GameRunner::addRegistryKey(const Profile &settings, const QString &key, const QString &value, const QString &data)
{
    auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(settings,
                     process,
                     {QStringLiteral("reg"), QStringLiteral("add"), key, QStringLiteral("/v"), value, QStringLiteral("/d"), data, QStringLiteral("/f")},
                     false,
                     false);
    process->waitForFinished();
}

void GameRunner::setWindowsVersion(const Profile &settings, const QString &version)
{
    auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(settings, process, {QStringLiteral("winecfg"), QStringLiteral("/v"), version}, false, false);
    process->waitForFinished();
}