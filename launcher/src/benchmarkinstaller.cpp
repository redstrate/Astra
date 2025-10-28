// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "benchmarkinstaller.h"

#include <KArchiveDirectory>
#include <KLocalizedString>
#include <KZip>

#include "astra_log.h"
#include "launchercore.h"
#include "profile.h"
#include "profileconfig.h"
#include "utility.h"

BenchmarkInstaller::BenchmarkInstaller(LauncherCore &launcher, Profile &profile, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
    , m_profile(profile)
{
}

BenchmarkInstaller::BenchmarkInstaller(LauncherCore &launcher, Profile &profile, const QString &filePath, QObject *parent)
    : BenchmarkInstaller(launcher, profile, parent)
{
    m_localInstallerPath = filePath;
}

void BenchmarkInstaller::start()
{
    if (m_localInstallerPath.isEmpty()) {
        const auto request = QNetworkRequest(QUrl(m_launcher.config()->benchmarkURL()));
        Utility::printRequest(QStringLiteral("GET"), request);

        auto reply = m_launcher.mgr()->get(request);

        connect(reply, &QNetworkReply::downloadProgress, this, [this](const qint64 received, const qint64 total) {
            m_downloadedBytes = received;
            Q_EMIT downloadedBytesChanged();

            m_totalBytes = total;
            Q_EMIT totalBytesChanged();
        });

        connect(reply, &QNetworkReply::finished, [this, reply] {
            if (reply->error() != QNetworkReply::NetworkError::NoError) {
                Q_EMIT error(reply->errorString());
                return;
            }

            const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

            const QByteArray data = reply->readAll();

            QFile file(dataDir.absoluteFilePath(QStringLiteral("ffxiv-bench.zip")));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();

            m_localInstallerPath = file.fileName();
            installGame();
        });
    } else {
        installGame();
    }
}

qint64 BenchmarkInstaller::totalBytes() const
{
    return m_totalBytes;
}

qint64 BenchmarkInstaller::downloadedBytes() const
{
    return m_downloadedBytes;
}

void BenchmarkInstaller::installGame()
{
    const QDir installDirectory = m_profile.config()->gamePath();

    KZip archive(m_localInstallerPath);
    if (!archive.open(QIODevice::ReadOnly)) {
        qCritical(ASTRA_LOG) << "Failed to extract benchmark files:" << archive.errorString();
        Q_EMIT error(i18n("Failed to extract benchmark files:\n\n%1", archive.errorString()));
        return;
    }

    // the first directory is the same as the version we download
    const KArchiveDirectory *root = archive.directory();
    Q_UNUSED(root->copyTo(installDirectory.absolutePath(), true))

    archive.close();

    m_profile.config()->setIsBenchmark(true);
    m_profile.readGameVersion();

    Q_EMIT installFinished();
    qInfo(ASTRA_LOG) << "Installed game in" << installDirectory;
}

#include "moc_benchmarkinstaller.cpp"
