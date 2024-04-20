// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "benchmarkinstaller.h"

#include <KArchiveDirectory>
#include <KLocalizedString>
#include <KZip>
#include <QFile>
#include <QNetworkReply>
#include <QStandardPaths>

#include "astra_log.h"
#include "launchercore.h"
#include "profile.h"
#include "utility.h"

// TODO: this should be dynamically grabbed from the webpage to avoid hardcoding it
const QString installerUrl = QStringLiteral("https://download.finalfantasyxiv.com/s9qmq6SJfMMqYM4o/ffxiv-dawntrail-bench.zip");

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
        const QNetworkRequest request = QNetworkRequest(QUrl(installerUrl));
        Utility::printRequest(QStringLiteral("GET"), request);

        // TODO: benchmarks are usually quite large, and need download progress reporting
        auto reply = m_launcher.mgr()->get(request);

        QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
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

void BenchmarkInstaller::installGame()
{
    const QDir installDirectory = m_profile.gamePath();

    KZip archive(m_localInstallerPath);
    if (!archive.open(QIODevice::ReadOnly)) {
        qCritical(ASTRA_LOG) << "Failed to extract benchmark files";
        Q_EMIT error(i18n("Failed to extract benchmark files."));
        return;
    }

    // the first directory is the same as the version we download
    const KArchiveDirectory *root = archive.directory();
    root->copyTo(installDirectory.absolutePath(), true);

    archive.close();

    m_profile.readGameVersion();

    Q_EMIT installFinished();
    qInfo(ASTRA_LOG) << "Installed game in" << installDirectory;
}

#include "moc_benchmarkinstaller.cpp"