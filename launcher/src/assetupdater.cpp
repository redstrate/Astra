// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "assetupdater.h"
#include "astra_log.h"
#include "profileconfig.h"
#include "utility.h"

#include <KLocalizedString>
#include <KTar>
#include <KZip>
#include <QCoroSignal>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUrlQuery>
#include <qcorofuture.h>
#include <qcoronetworkreply.h>

using namespace Qt::StringLiterals;

AssetUpdater::AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , launcher(launcher)
    , m_profile(profile)
{
}

QCoro::Task<bool> AssetUpdater::update()
{
    if (!m_profile.config()->dalamudEnabled()) {
        co_return true;
    }

    qInfo(ASTRA_LOG) << "Checking for asset updates...";

    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    m_dalamudDir = dataDir.absoluteFilePath(QStringLiteral("dalamud"));
    Utility::createPathIfNeeded(m_dalamudDir);

    m_dalamudAssetDir = m_dalamudDir.absoluteFilePath(QStringLiteral("assets"));
    Utility::createPathIfNeeded(m_dalamudAssetDir);

    m_dalamudRuntimeDir = m_dalamudDir.absoluteFilePath(QStringLiteral("runtime"));
    Utility::createPathIfNeeded(m_dalamudRuntimeDir);

    if (m_profile.config()->dalamudChannel() != QStringLiteral("local")) {
        if (!co_await checkRemoteDalamudAssetVersion()) {
            co_return false;
        }

        if (!co_await checkRemoteDalamudVersion()) {
            co_return false;
        }
    } else {
        qInfo(ASTRA_LOG) << "Using a local Dalamud installation, skipping version checks!";
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
        Q_EMIT launcher.dalamudError(i18n("Could not check for Dalamud asset updates:\n\n%1", reply->errorString()));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::dalamudDecided);
        co_return shouldContinue;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

    m_remoteDalamudAssetVersion = doc.object()["version"_L1].toInt();
    m_remoteDalamudAssetPackageUrl = doc.object()["packageUrl"_L1].toString();
    m_remoteDalamudAssetArray = doc.object()["assets"_L1].toArray();

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
    query.addQueryItem(QStringLiteral("track"), m_profile.config()->dalamudChannel());

    url.setQuery(query);

    const QNetworkRequest request(url);
    Utility::printRequest(QStringLiteral("GET"), request);

    m_remoteDalamudVersion.clear();
    m_remoteRuntimeVersion.clear();

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.dalamudError(i18n("Could not check for Dalamud updates:\n\n%1", reply->errorString()));

        // Always assume Dalamud is applicable for this version when we can't contact the server!
        // The user is prompted to turn off Dalamud anyway, if they wish.
        m_profile.setDalamudApplicable(true);

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::dalamudDecided);
        co_return shouldContinue;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    m_remoteDalamudVersion = doc["assemblyVersion"_L1].toString();
    m_remoteRuntimeVersion = doc["runtimeVersion"_L1].toString();
    m_remoteDalamudDownloadUrl = doc["downloadUrl"_L1].toString();

    // This is because sometimes isApplicableForCurrentGameVer is null for some reason.
    if (doc.object().contains("isApplicableForCurrentGameVer"_L1)) {
        m_profile.setDalamudApplicable(doc["isApplicableForCurrentGameVer"_L1].toBool());
    } else {
        m_profile.setDalamudApplicable(doc["supportedGameVer"_L1].toString() == m_profile.baseGameVersion());
    }

    m_profile.setDalamudApplicable(true);

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

    const auto request = QNetworkRequest(QUrl(m_remoteDalamudAssetPackageUrl));
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    qInfo(ASTRA_LOG) << "Finished downloading Dalamud assets";

    QFile file(m_tempDir.filePath(QStringLiteral("dalamud-assets.zip")));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
    } else {
        qCWarning(ASTRA_LOG) << "Could not open" << file.fileName() << "for writing:" << file.errorString();
    }

    if (const QString errorString = extractZip(m_tempDir.filePath(QStringLiteral("dalamud-assets.zip")), m_dalamudAssetDir.absolutePath());
        !errorString.isEmpty()) {
        qCritical(ASTRA_LOG) << "Failed to install Dalamud assets:" << errorString;
        Q_EMIT launcher.dalamudError(i18n("Failed to install Dalamud assets:\n\n%1").arg(errorString));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::dalamudDecided);
        co_return shouldContinue;
    }

    // TODO: check for file hashes

    m_profile.setDalamudAssetVersion(m_remoteDalamudAssetVersion);

    Utility::writeVersion(m_dalamudAssetDir.absoluteFilePath(QStringLiteral("asset.ver")), QString::number(m_remoteDalamudAssetVersion));

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installDalamud()
{
    Q_EMIT launcher.stageChanged(i18n("Updating Dalamud..."));

    const auto request = QNetworkRequest(QUrl(m_remoteDalamudDownloadUrl));
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    qInfo(ASTRA_LOG) << "Finished downloading Dalamud";

    QFile file(m_tempDir.filePath(QStringLiteral("latest.zip")));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
    } else {
        qCWarning(ASTRA_LOG) << "Could not open" << file.fileName() << "for writing:" << file.errorString();
    }

    if (const QString errorString =
            extractZip(m_tempDir.filePath(QStringLiteral("latest.zip")), m_dalamudDir.absoluteFilePath(m_profile.config()->dalamudChannel()));
        !errorString.isEmpty()) {
        qCritical(ASTRA_LOG) << "Failed to install Dalamud:" << errorString;
        Q_EMIT launcher.dalamudError(i18n("Failed to install Dalamud:\n\n%1").arg(errorString));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::dalamudDecided);
        co_return shouldContinue;
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
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
        } else {
            qCWarning(ASTRA_LOG) << "Could not open" << file.fileName() << "for writing:" << file.errorString();
        }
    }

    // desktop
    {
        const QNetworkRequest request(dotnetDesktopPackageUrl(m_remoteRuntimeVersion));
        Utility::printRequest(QStringLiteral("GET"), request);

        const auto reply = launcher.mgr()->get(request);
        co_await reply;

        qInfo(ASTRA_LOG) << "Finished downloading Dotnet-desktop";

        QFile file(m_tempDir.filePath(QStringLiteral("dotnet-desktop.zip")));
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
        } else {
            qCWarning(ASTRA_LOG) << "Could not open" << file.fileName() << "for writing:" << file.errorString();
        }
    }

    const QString errorString1 = extractZip(m_tempDir.filePath(QStringLiteral("dotnet-core.zip")), m_dalamudRuntimeDir.absolutePath());
    const QString errorString2 = extractZip(m_tempDir.filePath(QStringLiteral("dotnet-desktop.zip")), m_dalamudRuntimeDir.absolutePath());

    if (!errorString1.isEmpty() || !errorString2.isEmpty()) {
        qCritical(ASTRA_LOG) << "Failed to install .NET runtime:" << errorString1 << errorString2;
        Q_EMIT launcher.dalamudError(i18n("Failed to install .NET runtime:\n\n%1%2").arg(errorString1).arg(errorString2));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::dalamudDecided);
        co_return shouldContinue;
    } else {
        Utility::writeVersion(m_dalamudRuntimeDir.absoluteFilePath(QStringLiteral("runtime.ver")), m_remoteRuntimeVersion);

        co_return true;
    }
}

QUrl AssetUpdater::dalamudVersionManifestUrl() const
{
    QUrl url(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/VersionInfo"));

    return url;
}

QUrl AssetUpdater::dalamudAssetManifestUrl() const
{
    QUrl url(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Asset/Meta"));

    return url;
}

QUrl AssetUpdater::dotnetRuntimePackageUrl(const QString &version) const
{
    QUrl url(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/Runtime/DotNet/%1").arg(version));

    return url;
}

QUrl AssetUpdater::dotnetDesktopPackageUrl(const QString &version) const
{
    QUrl url(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/Runtime/WindowsDesktop/%1").arg(version));

    return url;
}

QString AssetUpdater::extractZip(const QString &filePath, const QString &directory)
{
    KZip archive(filePath);
    if (!archive.open(QIODevice::ReadOnly)) {
        return archive.errorString();
    }

    const KArchiveDirectory *root = archive.directory();
    if (!root->copyTo(directory, true)) {
        return archive.errorString();
    }

    return {};
}

#include "moc_assetupdater.cpp"
