// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profile.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "account.h"
#include "launchercore.h"

Profile::Profile(LauncherCore &launcher, const QString &key, QObject *parent)
    : QObject(parent)
    , m_uuid(key)
    , m_config(key)
    , m_launcher(launcher)
{
    readGameVersion();
    readWineInfo();

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const bool hasDalamud = QFile::exists(dataDir + "/Dalamud");
    if (hasDalamud) {
        if (QFile::exists(dataDir + "/Dalamud/Dalamud.deps.json")) {
            QFile depsJson(dataDir + "/Dalamud/Dalamud.deps.json");
            depsJson.open(QFile::ReadOnly);
            QJsonDocument doc = QJsonDocument::fromJson(depsJson.readAll());

            QString versionString;
            for (auto target : doc["targets"].toObject().keys()) {
                if (target.contains(".NETCoreApp")) {
                    versionString = doc["targets"].toObject()[target].toObject().keys().filter("Dalamud/")[0];
                }
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
}

void Profile::readGameData()
{
    physis_EXH *exh = physis_gamedata_read_excel_sheet_header(gameData, "ExVersion");
    if (exh != nullptr) {
        physis_EXD exd = physis_gamedata_read_excel_sheet(gameData, "ExVersion", exh, Language::English, 0);

        for (unsigned int i = 0; i < exd.row_count; i++) {
            expansionNames.push_back(exd.row_data[i].column_data[0].string._0);
        }

        physis_gamedata_free_sheet(exd);
        physis_gamedata_free_sheet_header(exh);
    }
}

void Profile::readWineInfo()
{
#if defined(Q_OS_MAC)
    switch (wineType) {
    case WineType::System: // system wine
        winePath = "/usr/local/bin/wine64";
        break;
    case WineType::Custom: // custom path
        winePath = profile.winePath;
        break;
    case WineType::Builtin: // ffxiv built-in (for mac users)
        winePath =
            "/Applications/FINAL FANTASY XIV "
            "ONLINE.app/Contents/SharedSupport/finalfantasyxiv/FINAL FANTASY XIV ONLINE/wine";
        break;
    case WineType::XIVOnMac:
        winePath = "/Applications/XIV on Mac.app/Contents/Resources/wine/bin/wine64";
        break;
    }
#endif

#if defined(Q_OS_LINUX)
    switch (wineType()) {
    case WineType::System: // system wine (should be in $PATH)
        setWinePath("wine");
        break;
    case WineType::Custom: // custom pth
    default:
        break;
    }
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    auto wineProcess = new QProcess(this);
    wineProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(wineProcess, &QProcess::readyRead, this, [wineProcess, this] {
        m_wineVersion = wineProcess->readAllStandardOutput().trimmed();
        Q_EMIT wineVersionText();
    });

    m_launcher.launchExecutable(*this, wineProcess, {"--version"}, false, false);

    wineProcess->waitForFinished();
#endif
}

QString Profile::name() const
{
    return m_config.name();
}

void Profile::setName(const QString &name)
{
    if (m_config.name() != name) {
        m_config.setName(name);
        m_config.save();
        Q_EMIT nameChanged();
    }
}

int Profile::language() const
{
    return m_config.language();
}

void Profile::setLanguage(const int value)
{
    if (m_config.language() != value) {
        m_config.setLanguage(value);
        m_config.save();
        Q_EMIT languageChanged();
    }
}

QString Profile::gamePath() const
{
    return m_config.gamePath();
}

void Profile::setGamePath(const QString &path)
{
    if (m_config.gamePath() != path) {
        m_config.setGamePath(path);
        m_config.save();
        Q_EMIT gamePathChanged();
    }
}

QString Profile::winePath() const
{
    return m_config.winePath();
}

void Profile::setWinePath(const QString &path)
{
    if (m_config.winePath() != path) {
        m_config.setWinePath(path);
        m_config.save();
        Q_EMIT winePathChanged();
    }
}

QString Profile::winePrefixPath() const
{
    return m_config.winePrefixPath();
}

void Profile::setWinePrefixPath(const QString &path)
{
    if (m_config.winePrefixPath() != path) {
        m_config.setWinePrefixPath(path);
        m_config.save();
        Q_EMIT winePrefixPathChanged();
    }
}

bool Profile::watchdogEnabled() const
{
    return m_config.enableWatchdog();
}

void Profile::setWatchdogEnabled(const bool value)
{
    if (m_config.enableWatchdog() != value) {
        m_config.setEnableWatchdog(value);
        m_config.save();
        Q_EMIT enableWatchdogChanged();
    }
}

Profile::WineType Profile::wineType() const
{
    return static_cast<WineType>(m_config.wineType());
}

void Profile::setWineType(const WineType type)
{
    if (static_cast<WineType>(m_config.wineType()) != type) {
        m_config.setWineType(static_cast<int>(type));
        m_config.save();
        Q_EMIT wineTypeChanged();
        readWineInfo();
    }
}

bool Profile::esyncEnabled() const
{
    return m_config.useESync();
}

void Profile::setESyncEnabled(const bool value)
{
    if (m_config.useESync() != value) {
        m_config.setUseESync(value);
        m_config.save();
        Q_EMIT useESyncChanged();
    }
}

bool Profile::gamescopeEnabled() const
{
    return m_config.useGamescope();
}

void Profile::setGamescopeEnabled(const bool value)
{
    if (m_config.useGamescope() != value) {
        m_config.setUseGamescope(value);
        m_config.save();
        Q_EMIT useGamescopeChanged();
    }
}

bool Profile::gamemodeEnabled() const
{
    return m_config.useGamemode();
}

void Profile::setGamemodeEnabled(const bool value)
{
    if (m_config.useGamemode() != value) {
        m_config.setUseGamemode(value);
        m_config.save();
        Q_EMIT useGamemodeChanged();
    }
}

bool Profile::directx9Enabled() const
{
    return m_config.useDX9();
}

void Profile::setDirectX9Enabled(const bool value)
{
    if (m_config.useDX9() != value) {
        m_config.setUseDX9(value);
        m_config.save();
        Q_EMIT useDX9Changed();
    }
}

bool Profile::gamescopeFullscreen() const
{
    return m_config.gamescopeFullscreen();
}

void Profile::setGamescopeFullscreen(const bool value)
{
    if (m_config.gamescopeFullscreen() != value) {
        m_config.setGamescopeFullscreen(value);
        m_config.save();
        Q_EMIT gamescopeFullscreenChanged();
    }
}

bool Profile::gamescopeBorderless() const
{
    return m_config.gamescopeBorderless();
}

void Profile::setGamescopeBorderless(const bool value)
{
    if (m_config.gamescopeBorderless() != value) {
        m_config.setGamescopeBorderless(value);
        m_config.save();
        Q_EMIT gamescopeBorderlessChanged();
    }
}

int Profile::gamescopeWidth() const
{
    return m_config.gamescopeWidth();
}

void Profile::setGamescopeWidth(const int value)
{
    if (m_config.gamescopeWidth() != value) {
        m_config.setGamescopeWidth(value);
        m_config.save();
        Q_EMIT gamescopeWidthChanged();
    }
}

int Profile::gamescopeHeight() const
{
    return m_config.gamescopeHeight();
}

void Profile::setGamescopeHeight(const int value)
{
    if (m_config.gamescopeHeight() != value) {
        m_config.setGamescopeHeight(value);
        m_config.save();
        Q_EMIT gamescopeHeightChanged();
    }
}

int Profile::gamescopeRefreshRate() const
{
    return m_config.gamescopeRefreshRate();
}

void Profile::setGamescopeRefreshRate(const int value)
{
    if (m_config.gamescopeRefreshRate() != value) {
        m_config.setGamescopeRefreshRate(value);
        m_config.save();
        Q_EMIT gamescopeRefreshRateChanged();
    }
}

bool Profile::dalamudEnabled() const
{
    return m_config.dalamudEnabled();
}

void Profile::setDalamudEnabled(const bool value)
{
    if (m_config.dalamudEnabled() != value) {
        m_config.setDalamudEnabled(value);
        m_config.save();
        Q_EMIT dalamudEnabledChanged();
    }
}

bool Profile::dalamudOptOut() const
{
    return m_config.dalamudOptOut();
}

void Profile::setDalamudOptOut(const bool value)
{
    if (m_config.dalamudOptOut() != value) {
        m_config.setDalamudOptOut(value);
        m_config.save();
        Q_EMIT dalamudOptOutChanged();
    }
}

Profile::DalamudChannel Profile::dalamudChannel() const
{
    return static_cast<DalamudChannel>(m_config.dalamudChannel());
}

void Profile::setDalamudChannel(const DalamudChannel value)
{
    if (static_cast<DalamudChannel>(m_config.dalamudChannel()) != value) {
        m_config.setDalamudChannel(static_cast<int>(value));
        m_config.save();
        Q_EMIT dalamudChannelChanged();
    }
}

bool Profile::argumentsEncrypted() const
{
    return m_config.encryptArguments();
}

void Profile::setArgumentsEncrypted(const bool value)
{
    if (m_config.encryptArguments() != value) {
        m_config.setEncryptArguments(value);
        m_config.save();
        Q_EMIT encryptedArgumentsChanged();
    }
}

Account *Profile::account() const
{
    return m_account;
}

void Profile::setAccount(Account *account)
{
    if (account != m_account) {
        m_account = account;
        if (account->uuid() != m_config.account()) {
            m_config.setAccount(account->uuid());
            m_config.save();
        }
        Q_EMIT accountChanged();
    }
}

void Profile::readGameVersion()
{
    if (gamePath().isEmpty()) {
        return;
    }

    gameData = physis_gamedata_initialize((gamePath() + "/game").toStdString().c_str());
    bootData = physis_bootdata_initialize((gamePath() + "/boot").toStdString().c_str());

    if (bootData != nullptr) {
        bootVersion = physis_bootdata_get_version(bootData);
    }

    if (gameData != nullptr) {
        repositories = physis_gamedata_get_repositories(gameData);
        readGameData();
    }

    Q_EMIT gameInstallChanged();
}

QString Profile::accountUuid() const
{
    return m_config.account();
}

QString Profile::expansionVersionText() const
{
    if (!isGameInstalled()) {
        return "No game installed.";
    } else {
        QString expacString;

        expacString += "Boot";
        expacString += QString(" (%1)").arg(bootVersion);

        for (unsigned int i = 0; i < repositories.repositories_count; i++) {
            QString expansionName = "Unknown Expansion";
            if (i < static_cast<unsigned int>(expansionNames.size())) {
                expansionName = expansionNames[i];
            }

            expacString += QString("\n%1 (%2)").arg(expansionName, repositories.repositories[i].version);
        }

        return expacString;
    }
}

QString Profile::dalamudVersionText() const
{
    QString text;
    if (dalamudVersion.isEmpty()) {
        text += "Dalamud is not installed.";
    } else {
        text += QStringLiteral("Dalamud (%1)").arg(dalamudVersion);
    }

    if (dalamudAssetVersion != -1) {
        text += QStringLiteral("\nAssets (%1)").arg(QString::number(dalamudAssetVersion));
    }

    return text;
}

QString Profile::uuid() const
{
    return m_uuid;
}

QString Profile::wineVersionText() const
{
    if (m_launcher.isSteam()) {
        return "Wine is being managed by Steam.";
    }

    if (!isWineInstalled()) {
        return "Wine is not installed.";
    } else {
        return m_wineVersion;
    }
}
