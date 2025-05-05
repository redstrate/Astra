// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profile.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonDocument>
#include <QProcess>

#include "account.h"
#include "astra_log.h"
#include "launchercore.h"
#include "profileconfig.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Profile::Profile(const QString &key, QObject *parent)
    : QObject(parent)
    , m_uuid(key)
    , m_config(new ProfileConfig(key, this))
{
    readGameVersion();
    readWineInfo();
    readDalamudInfo();

    connect(m_config, &ProfileConfig::WineTypeChanged, this, &Profile::readWineInfo);
    connect(m_config, &ProfileConfig::GamePathChanged, this, &Profile::readGameVersion);
    connect(m_config, &ProfileConfig::GamePathChanged, this, &Profile::hasDirectx9Changed);
}

Profile::~Profile()
{
    m_config->save();
}

void Profile::readDalamudInfo()
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const QDir compatibilityToolDir = dataDir.absoluteFilePath(QStringLiteral("tool"));
    const QDir wineDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("wine"));
    if (wineDir.exists()) {
        const QString wineVer = wineDir.absoluteFilePath(QStringLiteral("wine.ver"));
        if (QFile::exists(wineVer)) {
            m_compatibilityToolVersion = Utility::readVersion(wineVer);
            qInfo(ASTRA_LOG) << "Compatibility tool version:" << m_compatibilityToolVersion;
        }
    }

    const QDir dalamudDir = dataDir.absoluteFilePath(QStringLiteral("dalamud"));

    if (dalamudDir.exists()) {
        const QDir dalamudInstallDir = dalamudDir.absoluteFilePath(dalamudChannelName());
        const QDir dalamudAssetsDir = dalamudDir.absoluteFilePath(QStringLiteral("assets"));
        const QDir dalamudRuntimeDir = dalamudDir.absoluteFilePath(QStringLiteral("runtime"));

        const QString dalamudDepsJson = dalamudInstallDir.absoluteFilePath(QStringLiteral("Dalamud.deps.json"));
        if (QFile::exists(dalamudDepsJson)) {
            QFile depsJson(dalamudDepsJson);
            depsJson.open(QFile::ReadOnly);
            const QJsonDocument doc = QJsonDocument::fromJson(depsJson.readAll());

            QString versionString;
            for (const auto &target : doc["targets"_L1].toObject().keys()) {
                if (target.contains(".NETCoreApp"_L1)) {
                    versionString = doc["targets"_L1].toObject()[target].toObject().keys().filter(QStringLiteral("Dalamud/"))[0];
                }
            }

            m_dalamudVersion = versionString.remove("Dalamud/"_L1);
            qInfo(ASTRA_LOG) << "Dalamud version:" << m_dalamudVersion;
        }

        const QString dalamudAssetsVer = dalamudAssetsDir.absoluteFilePath(QStringLiteral("asset.ver"));
        if (QFile::exists(dalamudAssetsVer)) {
            m_dalamudAssetVersion = Utility::readVersion(dalamudAssetsVer).toInt();
            qInfo(ASTRA_LOG) << "Dalamud asset version:" << m_dalamudVersion;
        }

        const QString dalamudRuntimeVer = dalamudRuntimeDir.absoluteFilePath(QStringLiteral("runtime.ver"));
        if (QFile::exists(dalamudRuntimeVer)) {
            m_runtimeVersion = Utility::readVersion(dalamudRuntimeVer);
            qInfo(ASTRA_LOG) << "Dalamud runtime version:" << m_dalamudVersion;
        }
    }
}

void Profile::readGameData()
{
    if (!physis_gamedata_exists(m_gameData, "exd/exversion.exh")) {
        return;
    }

    const auto header = physis_gamedata_extract_file(m_gameData, "exd/exversion.exh");
    physis_EXH *exh = physis_parse_excel_sheet_header(header);
    if (exh != nullptr) {
        const physis_EXD exd = physis_gamedata_read_excel_sheet(m_gameData, "ExVersion", exh, Language::English, 0);

        // TODO: bad API, we should instead get a list of row ids from libphysis but that API doesn't exist yet.
        for (unsigned int i = 0; i < exd.row_count; i++) {
            auto rows = physis_exd_read_row(&exd, i);
            for (int j = 0; j < rows.row_count; j++) {
                auto row = rows.row_data[j];
                if (row.column_data != nullptr) {
                    m_expansionNames.push_back(QString::fromLatin1(row.column_data[0].string._0));
                }
            }
        }

        physis_gamedata_free_sheet(exd);
        physis_gamedata_free_sheet_header(exh);
    }
}

void Profile::readWineInfo()
{
    auto wineProcess = new QProcess(this);

    connect(wineProcess, &QProcess::readyReadStandardOutput, this, [wineProcess, this] {
        m_wineVersion = QString::fromUtf8(wineProcess->readAllStandardOutput().trimmed());
        Q_EMIT wineChanged();
    });

    wineProcess->start(winePath(), {QStringLiteral("--version")});
    wineProcess->waitForFinished();
}

QString Profile::winePath() const
{
    switch (config()->wineType()) {
    case WineType::BuiltIn: {
        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir compatibilityToolDir = dataDir.absoluteFilePath(QStringLiteral("tool"));
        const QDir wineDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("wine"));
        const QDir wineBinDir = wineDir.absoluteFilePath(QStringLiteral("bin"));

        return wineBinDir.absoluteFilePath(QStringLiteral("wine64"));
    }
    case WineType::Custom: // custom pth
        return m_config->winePath();
    default:
        return {};
    }
}

void Profile::setWinePath(const QString &path)
{
    if (m_config->winePath() != path) {
        m_config->setWinePath(path);
        Q_EMIT winePathChanged();
    }
}

bool Profile::hasDirectx9() const
{
    const QDir gameDir(config()->gamePath());
    return QFileInfo::exists(gameDir.absoluteFilePath(QStringLiteral("game/ffxiv.exe")));
}

Account *Profile::account() const
{
    return m_account;
}

void Profile::setAccount(Account *account)
{
    m_account = account;
    m_config->setAccount(account->uuid());
    Q_EMIT accountChanged();
}

void Profile::readGameVersion()
{
    if (config()->gamePath().isEmpty()) {
        return;
    }

    m_gameData = physis_gamedata_initialize(QString(config()->gamePath() + QStringLiteral("/game")).toStdString().c_str());
    m_bootData = physis_bootdata_initialize(QString(config()->gamePath() + QStringLiteral("/boot")).toStdString().c_str());

    if (m_bootData != nullptr) {
        m_bootVersion = physis_bootdata_get_version(m_bootData);
    }

    if (m_gameData != nullptr) {
        m_repositories = physis_gamedata_get_repositories(m_gameData);
        readGameData();
    }

    // Extract frontier url if possible
    const auto launcherPath = QString(config()->gamePath() + QStringLiteral("/boot/ffxivlauncher64.exe"));
    if (QFile::exists(launcherPath)) {
        m_frontierUrl = QString::fromUtf8(physis_extract_frontier_url(launcherPath.toStdString().c_str()));
    }

    Q_EMIT gameInstallChanged();
}

QString Profile::expansionVersionText() const
{
    if (!isGameInstalled()) {
        return i18n("No game installed.");
    } else {
        QString expacString;

        expacString += QStringLiteral("Boot");

        if (!m_bootVersion) {
            expacString += i18n(" (Not Installed)");
        } else {
            expacString += QStringLiteral(" (%1)").arg(QString::fromLatin1(m_bootVersion));
        }

        for (unsigned int i = 0; i < m_repositories.repositories_count; i++) {
            QString expansionName = i18n("Unknown Expansion");
            if (i < static_cast<unsigned int>(m_expansionNames.size())) {
                expansionName = m_expansionNames[i];
            }

            expacString += QStringLiteral("\n%1 (%2)").arg(expansionName, QString::fromLatin1(m_repositories.repositories[i].version));
        }

        return expacString;
    }
}

QString Profile::dalamudVersionText() const
{
    QString text;
    if (m_dalamudVersion.isEmpty()) {
        text += i18n("Dalamud is not installed.");
    } else {
        text += QStringLiteral("Dalamud (%1)").arg(m_dalamudVersion);
    }

    if (m_dalamudAssetVersion != -1) {
        text += QStringLiteral("\nAssets (%1)").arg(QString::number(m_dalamudAssetVersion));
    }

    return text;
}

QString Profile::uuid() const
{
    return m_uuid;
}

QString Profile::wineVersionText() const
{
    if (!isWineInstalled()) {
        return i18n("Wine is not installed.");
    } else {
        return m_wineVersion;
    }
}

QString Profile::dalamudChannelName() const
{
    switch (config()->dalamudChannel()) {
    case DalamudChannel::Stable:
        return QStringLiteral("stable");
    case DalamudChannel::Staging:
        return QStringLiteral("stg");
    case DalamudChannel::Local:
        return QStringLiteral("local");
    }

    Q_UNREACHABLE();
}

[[nodiscard]] bool Profile::isGameInstalled() const
{
    return m_repositories.repositories_count > 0;
}

[[nodiscard]] bool Profile::isWineInstalled() const
{
    return !m_wineVersion.isEmpty();
}

QString Profile::bootVersion() const
{
    return QString::fromLatin1(m_bootVersion);
}

QString Profile::baseGameVersion() const
{
    Q_ASSERT(m_repositories.repositories_count >= 1);
    if (m_repositories.repositories_count == 0 || m_repositories.repositories[0].version == nullptr) {
        return QStringLiteral("2012.01.01.0000.0000");
    }
    return QString::fromLatin1(m_repositories.repositories[0].version);
}

int Profile::numInstalledExpansions() const
{
    Q_ASSERT(m_repositories.repositories_count >= 1);
    return static_cast<int>(m_repositories.repositories_count) - 1;
}

QString Profile::expansionVersion(const int index) const
{
    Q_ASSERT(index <= numInstalledExpansions());
    return QString::fromLatin1(m_repositories.repositories[index + 1].version);
}

QString Profile::frontierUrl() const
{
    if (m_frontierUrl.isEmpty()) {
        // fallback url
        return QStringLiteral("https://launcher.finalfantasyxiv.com/v700/");
    } else {
        return m_frontierUrl;
    }
}

int Profile::dalamudAssetVersion() const
{
    return m_dalamudAssetVersion;
}

void Profile::setDalamudAssetVersion(const int version)
{
    m_dalamudAssetVersion = version;
}

QString Profile::runtimeVersion() const
{
    return m_runtimeVersion;
}

QString Profile::dalamudVersion() const
{
    return m_dalamudVersion;
}

void Profile::setDalamudVersion(const QString &version)
{
    m_dalamudVersion = version;
}

void Profile::setDalamudApplicable(const bool applicable)
{
    m_dalamudApplicable = applicable;
}

bool Profile::dalamudShouldLaunch() const
{
    // Local Dalamud installations can always run
    return config()->dalamudEnabled() && (config()->dalamudChannel() != DalamudChannel::Local ? m_dalamudApplicable : true);
}

QString Profile::compatibilityToolVersion() const
{
    return m_compatibilityToolVersion;
}

void Profile::setCompatibilityToolVersion(const QString &version)
{
    m_compatibilityToolVersion = version;
}

BootData *Profile::bootData() const
{
    return m_bootData;
}

GameData *Profile::gameData() const
{
    return m_gameData;
}

QString Profile::subtitle() const
{
    if (config()->isBenchmark()) {
        return i18n("Benchmark");
    } else if (m_repositories.repositories_count > 0) {
        const unsigned int latestExpansion = m_repositories.repositories_count - 1;
        QString expansionName = i18n("Unknown Expansion");
        if (latestExpansion < static_cast<unsigned int>(m_expansionNames.size())) {
            expansionName = m_expansionNames[latestExpansion];
        }

        return QStringLiteral("%1 (%2)").arg(expansionName, QString::fromLatin1(m_repositories.repositories[latestExpansion].version));
    } else {
        return i18n("Unknown");
    }
}

ProfileConfig *Profile::config() const
{
    return m_config;
}

#include "moc_profile.cpp"
