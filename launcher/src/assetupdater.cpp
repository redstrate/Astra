// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "assetupdater.h"
#include "astra_log.h"
#include "utility.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStandardPaths>
#include <qcorofuture.h>
#include <qcoronetworkreply.h>

#include <JlCompress.h>
#include <QtConcurrentRun>

AssetUpdater::AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , launcher(launcher)
    , m_profile(profile)
{
}

QCoro::Task<bool> AssetUpdater::update()
{
    if (!m_profile.dalamudEnabled()) {
        co_return true;
    }

    qInfo(ASTRA_LOG) << "Checking for asset updates...";

    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_dalamudDir = m_dataDir.absoluteFilePath(QStringLiteral("dalamud"));
    m_dalamudAssetDir = m_dalamudDir.absoluteFilePath(QStringLiteral("assets"));
    m_dalamudRuntimeDir = m_dalamudDir.absoluteFilePath(QStringLiteral("runtime"));

    const auto createIfNeeded = [](const QDir &dir) {
        if (!QDir().exists(dir.absolutePath()))
            QDir().mkpath(dir.absolutePath());
    };

    createIfNeeded(m_dalamudDir);
    createIfNeeded(m_dalamudAssetDir);
    createIfNeeded(m_dalamudRuntimeDir);

    if (!co_await checkRemoteDalamudAssetVersion()) {
        co_return false;
    }

    if (!co_await checkRemoteDalamudVersion()) {
        co_return false;
    }

    co_return true;
}

QCoro::Task<bool> AssetUpdater::checkRemoteDalamudAssetVersion()
{
    // first we want to fetch the list of assets required
    const QNetworkRequest request(dalamudAssetManifestUrl());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.dalamudError(i18n("Could not check for Dalamud asset updates.\n\n%1", reply->errorString()));
        co_return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

    m_remoteDalamudAssetVersion = doc.object()[QLatin1String("version")].toInt();
    m_remoteDalamudAssetArray = doc.object()[QLatin1String("assets")].toArray();

    qInfo(ASTRA_LOG) << "Dalamud asset remote version" << m_remoteDalamudAssetVersion;
    qInfo(ASTRA_LOG) << "Dalamud asset local version" << m_profile.dalamudAssetVersion();

    // dalamud assets
    if (m_remoteDalamudAssetVersion != m_profile.dalamudAssetVersion()) {
        qInfo(ASTRA_LOG) << "Dalamud assets out of date";

        co_return co_await installDalamudAssets();
    }

    co_return true;
}

QCoro::Task<bool> AssetUpdater::checkRemoteDalamudVersion()
{
    QUrl url(dalamudVersionManifestUrl());

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("track"), m_profile.dalamudChannelName());

    const QNetworkRequest request(url);
    Utility::printRequest(QStringLiteral("GET"), request);

    m_remoteDalamudVersion.clear();
    m_remoteRuntimeVersion.clear();

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.dalamudError(i18n("Could not check for Dalamud updates.\n\n%1", reply->errorString()));
        co_return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    m_remoteDalamudVersion = doc[QLatin1String("assemblyVersion")].toString();
    m_remoteRuntimeVersion = doc[QLatin1String("runtimeVersion")].toString();
    m_remoteDalamudDownloadUrl = doc[QLatin1String("downloadUrl")].toString();

    qInfo(ASTRA_LOG) << "Latest available Dalamud version:" << m_remoteDalamudVersion << "local:" << m_profile.dalamudVersion();
    qInfo(ASTRA_LOG) << "Latest available NET runtime:" << m_remoteRuntimeVersion;

    if (m_remoteDalamudVersion != m_profile.dalamudVersion()) {
        if (!co_await installDalamud()) {
            co_return false;
        }
    }

    if (m_profile.runtimeVersion() != m_remoteRuntimeVersion) {
        if (!co_await installRuntime()) {
            co_return false;
        }
    }

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installDalamudAssets()
{
    Q_EMIT launcher.stageChanged(i18n("Updating Dalamud assets..."));

    QFutureSynchronizer<void> synchronizer;

    for (const auto &assetObject : m_remoteDalamudAssetArray) {
        const QNetworkRequest assetRequest(assetObject.toObject()[QLatin1String("url")].toString());
        Utility::printRequest(QStringLiteral("GET"), assetRequest);

        const auto assetReply = launcher.mgr()->get(assetRequest);

        const auto future = QtFuture::connect(assetReply, &QNetworkReply::finished).then([this, assetReply, assetObject] {
            const QString fileName = assetObject.toObject()[QLatin1String("fileName")].toString();
            const QString dirPath = fileName.left(fileName.lastIndexOf(QLatin1Char('/')));

            const QString path = m_dalamudAssetDir.absoluteFilePath(dirPath);

            if (!QDir().exists(path))
                QDir().mkpath(path);

            QFile file(m_dalamudAssetDir.absoluteFilePath(assetObject.toObject()[QLatin1String("fileName")].toString()));
            file.open(QIODevice::WriteOnly);
            file.write(assetReply->readAll());
            file.close();
        });

        synchronizer.addFuture(future);
    }

    co_await QtConcurrent::run([&synchronizer] {
        synchronizer.waitForFinished();
    });

    qInfo(ASTRA_LOG) << "Finished downloading Dalamud assets";

    m_profile.setDalamudAssetVersion(m_remoteDalamudAssetVersion);

    QFile file(m_dalamudAssetDir.absoluteFilePath(QStringLiteral("asset.ver")));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(QString::number(m_remoteDalamudAssetVersion).toUtf8());
    file.close();

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installDalamud()
{
    Q_EMIT launcher.stageChanged(i18n("Updating Dalamud..."));

    const QNetworkRequest request(m_remoteDalamudDownloadUrl);
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    qInfo(ASTRA_LOG) << "Finished downloading Dalamud";

    QFile file(m_tempDir.filePath(QStringLiteral("/latest.zip")));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    const bool success =
        !JlCompress::extractDir(m_tempDir.filePath(QStringLiteral("latest.zip")), m_dalamudDir.absoluteFilePath(m_profile.dalamudChannelName())).empty();

    if (!success) {
        qCritical(ASTRA_LOG) << "Failed to install Dalamud";
        Q_EMIT launcher.dalamudError(i18n("Failed to install Dalamud."));
        co_return false;
    }

    m_profile.setDalamudVersion(m_remoteDalamudVersion);

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installRuntime()
{
    Q_EMIT launcher.stageChanged(i18n("Updating .NET Runtime..."));

    // core
    {
        const QNetworkRequest request(dotnetRuntimePackageUrl(m_remoteRuntimeVersion));
        Utility::printRequest(QStringLiteral("GET"), request);

        const auto reply = launcher.mgr()->get(request);
        co_await reply;

        qInfo(ASTRA_LOG) << "Finished downloading Dotnet-core";

        QFile file(m_tempDir.filePath(QStringLiteral("dotnet-core.zip")));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }

    // desktop
    {
        const QNetworkRequest request(dotnetDesktopPackageUrl(m_remoteRuntimeVersion));
        Utility::printRequest(QStringLiteral("GET"), request);

        const auto reply = launcher.mgr()->get(request);
        co_await reply;

        qInfo(ASTRA_LOG) << "Finished downloading Dotnet-desktop";

        QFile file(m_tempDir.filePath(QStringLiteral("dotnet-desktop.zip")));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }

    bool success = !JlCompress::extractDir(m_tempDir.filePath(QStringLiteral("dotnet-core.zip")), m_dalamudRuntimeDir.absolutePath()).empty();
    success |= !JlCompress::extractDir(m_tempDir.filePath(QStringLiteral("dotnet-desktop.zip")), m_dalamudRuntimeDir.absolutePath()).empty();

    if (!success) {
        qCritical(ASTRA_LOG) << "Failed to install dotnet";
        Q_EMIT launcher.dalamudError(i18n("Failed to install .NET runtime."));

        co_return false;
    } else {
        QFile file(m_dalamudRuntimeDir.absoluteFilePath(QStringLiteral("runtime.ver")));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(m_remoteRuntimeVersion.toUtf8());
        file.close();

        co_return true;
    }
}

QUrl AssetUpdater::dalamudVersionManifestUrl() const
{
    QUrl url;
    url.setScheme(launcher.settings()->preferredProtocol());
    url.setHost(launcher.settings()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/VersionInfo"));

    return url;
}

QUrl AssetUpdater::dalamudAssetManifestUrl() const
{
    QUrl url;
    url.setScheme(launcher.settings()->preferredProtocol());
    url.setHost(launcher.settings()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Asset/Meta"));

    return url;
}

QUrl AssetUpdater::dotnetRuntimePackageUrl(const QString &version) const
{
    QUrl url;
    url.setScheme(launcher.settings()->preferredProtocol());
    url.setHost(launcher.settings()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/Runtime/DotNet/%1").arg(version));

    return url;
}

QUrl AssetUpdater::dotnetDesktopPackageUrl(const QString &version) const
{
    QUrl url;
    url.setScheme(launcher.settings()->preferredProtocol());
    url.setHost(launcher.settings()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/Runtime/WindowsDesktop/%1").arg(version));

    return url;
}
