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
    , chosenChannel(profile.dalamudChannel())
    , m_profile(profile)
{
}

QCoro::Task<bool> AssetUpdater::update()
{
    if (!m_profile.dalamudEnabled()) {
        co_return true;
    }

    qInfo(ASTRA_LOG) << "Checking for asset updates...";

    dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dalamudDir = dataDir.absoluteFilePath(QStringLiteral("dalamud"));
    dalamudAssetDir = dalamudDir.absoluteFilePath(QStringLiteral("assets"));
    dalamudRuntimeDir = dalamudDir.absoluteFilePath(QStringLiteral("runtime"));

    const auto createIfNeeded = [](const QDir &dir) {
        if (!QDir().exists(dir.absolutePath()))
            QDir().mkpath(dir.absolutePath());
    };

    createIfNeeded(dalamudDir);
    createIfNeeded(dalamudAssetDir);
    createIfNeeded(dalamudRuntimeDir);

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

    remoteDalamudAssetVersion = doc.object()[QLatin1String("version")].toInt();
    remoteDalamudAssetArray = doc.object()[QLatin1String("assets")].toArray();

    qInfo(ASTRA_LOG) << "Dalamud asset remote version" << remoteDalamudAssetVersion;
    qInfo(ASTRA_LOG) << "Dalamud asset local version" << m_profile.dalamudAssetVersion();

    // dalamud assets
    if (remoteDalamudAssetVersion != m_profile.dalamudAssetVersion()) {
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

    remoteDalamudVersion.clear();
    remoteRuntimeVersion.clear();

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.dalamudError(i18n("Could not check for Dalamud updates.\n\n%1", reply->errorString()));
        co_return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    remoteDalamudVersion = doc[QLatin1String("assemblyVersion")].toString();
    remoteRuntimeVersion = doc[QLatin1String("runtimeVersion")].toString();
    remoteDalamudDownloadUrl = doc[QLatin1String("downloadUrl")].toString();

    qInfo(ASTRA_LOG) << "Latest available Dalamud version:" << remoteDalamudVersion << "local:" << m_profile.dalamudVersion();
    qInfo(ASTRA_LOG) << "Latest available NET runtime:" << remoteRuntimeVersion;

    if (remoteDalamudVersion != m_profile.dalamudVersion()) {
        if (!co_await installDalamud()) {
            co_return false;
        }
    }

    if (m_profile.runtimeVersion() != remoteRuntimeVersion) {
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

    for (const auto &assetObject : remoteDalamudAssetArray) {
        const QNetworkRequest assetRequest(assetObject.toObject()[QLatin1String("url")].toString());
        Utility::printRequest(QStringLiteral("GET"), assetRequest);

        const auto assetReply = launcher.mgr()->get(assetRequest);

        const auto future = QtFuture::connect(assetReply, &QNetworkReply::finished).then([this, assetReply, assetObject] {
            const QString fileName = assetObject.toObject()[QLatin1String("fileName")].toString();
            const QString dirPath = fileName.left(fileName.lastIndexOf(QLatin1Char('/')));

            const QString path = dalamudAssetDir.absoluteFilePath(dirPath);

            if (!QDir().exists(path))
                QDir().mkpath(path);

            QFile file(dalamudAssetDir.absoluteFilePath(assetObject.toObject()[QLatin1String("fileName")].toString()));
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

    m_profile.setDalamudAssetVersion(remoteDalamudAssetVersion);

    QFile file(dalamudAssetDir.absoluteFilePath(QStringLiteral("asset.ver")));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(QString::number(remoteDalamudAssetVersion).toUtf8());
    file.close();

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installDalamud()
{
    Q_EMIT launcher.stageChanged(i18n("Updating Dalamud..."));

    const QNetworkRequest request(remoteDalamudDownloadUrl);
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    qInfo(ASTRA_LOG) << "Finished downloading Dalamud";

    QFile file(tempDir.filePath(QStringLiteral("/latest.zip")));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    const bool success =
        !JlCompress::extractDir(tempDir.filePath(QStringLiteral("latest.zip")), dalamudDir.absoluteFilePath(m_profile.dalamudChannelName())).empty();

    if (!success) {
        qCritical(ASTRA_LOG) << "Failed to install Dalamud";
        Q_EMIT launcher.dalamudError(i18n("Failed to install Dalamud."));
        co_return false;
    }

    m_profile.setDalamudVersion(remoteDalamudVersion);

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installRuntime()
{
    Q_EMIT launcher.stageChanged(i18n("Updating .NET Runtime..."));

    // core
    {
        const QNetworkRequest request(dotnetRuntimePackageUrl(remoteRuntimeVersion));
        Utility::printRequest(QStringLiteral("GET"), request);

        const auto reply = launcher.mgr()->get(request);
        co_await reply;

        qInfo(ASTRA_LOG) << "Finished downloading Dotnet-core";

        QFile file(tempDir.filePath(QStringLiteral("dotnet-core.zip")));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }

    // desktop
    {
        const QNetworkRequest request(dotnetDesktopPackageUrl(remoteRuntimeVersion));
        Utility::printRequest(QStringLiteral("GET"), request);

        const auto reply = launcher.mgr()->get(request);
        co_await reply;

        qInfo(ASTRA_LOG) << "Finished downloading Dotnet-desktop";

        QFile file(tempDir.filePath(QStringLiteral("dotnet-desktop.zip")));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }

    bool success = !JlCompress::extractDir(tempDir.filePath(QStringLiteral("dotnet-core.zip")), dalamudRuntimeDir.absolutePath()).empty();
    success |= !JlCompress::extractDir(tempDir.filePath(QStringLiteral("dotnet-desktop.zip")), dalamudRuntimeDir.absolutePath()).empty();

    if (!success) {
        qCritical(ASTRA_LOG) << "Failed to install dotnet";
        Q_EMIT launcher.dalamudError(i18n("Failed to install .NET runtime."));

        co_return false;
    } else {
        QFile file(dalamudRuntimeDir.absoluteFilePath(QStringLiteral("runtime.ver")));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(remoteRuntimeVersion.toUtf8());
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
