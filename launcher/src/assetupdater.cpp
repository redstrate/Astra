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
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStandardPaths>
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
    qInfo(ASTRA_LOG) << "Checking for compatibility tool updates...";

    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (LauncherCore::needsCompatibilityTool()) {
        const QDir compatibilityToolDir = m_dataDir.absoluteFilePath(QStringLiteral("tool"));
        m_wineDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("wine"));
        m_dxvkDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("dxvk"));

        Utility::createPathIfNeeded(m_wineDir);
        Utility::createPathIfNeeded(m_dxvkDir);

        if (m_profile.config()->wineType() == Profile::WineType::BuiltIn && !co_await checkRemoteCompatibilityToolVersion()) {
            co_return false;
        }

        // TODO: should DXVK be tied to this setting...?
        if (m_profile.config()->wineType() == Profile::WineType::BuiltIn && !co_await checkRemoteDxvkVersion()) {
            co_return false;
        }
    }

    if (!m_profile.config()->dalamudEnabled()) {
        co_return true;
    }

    qInfo(ASTRA_LOG) << "Checking for asset updates...";

    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_dalamudDir = m_dataDir.absoluteFilePath(QStringLiteral("dalamud"));
    m_dalamudAssetDir = m_dalamudDir.absoluteFilePath(QStringLiteral("assets"));
    m_dalamudRuntimeDir = m_dalamudDir.absoluteFilePath(QStringLiteral("runtime"));

    Utility::createPathIfNeeded(m_dalamudDir);
    Utility::createPathIfNeeded(m_dalamudAssetDir);
    Utility::createPathIfNeeded(m_dalamudRuntimeDir);

    if (m_profile.config()->dalamudChannel() != Profile::DalamudChannel::Local) {
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

QCoro::Task<bool> AssetUpdater::checkRemoteCompatibilityToolVersion()
{
    const QNetworkRequest request(wineReleasesUrl());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await launcher.mgr()->get(request);
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.assetError(i18n("Could not check for the latest compatibility tool version.\n\n%1", reply->errorString()));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::assetDecided);
        co_return shouldContinue;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray releases = doc.array();
    const QJsonObject latestRelease = releases.first().toObject();

    if (latestRelease.contains("tag_name"_L1)) {
        m_remoteCompatibilityToolVersion = latestRelease["tag_name"_L1].toString();
    } else if (latestRelease.contains("name"_L1)) {
        m_remoteCompatibilityToolVersion = latestRelease["name"_L1].toString();
    } else {
        m_remoteCompatibilityToolVersion = latestRelease["commit"_L1].toObject()["sha"_L1].toString();
    }

    for (auto asset : latestRelease["assets"_L1].toArray()) {
        if (asset["name"_L1].toString().contains("ubuntu"_L1)) {
            m_remoteCompatibilityToolUrl = asset["browser_download_url"_L1].toString();
        }
    }

    // fallback to first asset just in case
    if (m_remoteCompatibilityToolUrl.isEmpty()) {
        m_remoteCompatibilityToolUrl = latestRelease["assets"_L1].toArray().first()["browser_download_url"_L1].toString();
    }

    qInfo(ASTRA_LOG) << "Compatibility tool remote version" << m_remoteCompatibilityToolVersion;

    // TODO: this version should not be profile specific
    if (m_remoteCompatibilityToolVersion != m_profile.compatibilityToolVersion()) {
        qInfo(ASTRA_LOG) << "Compatibility tool out of date";

        co_return co_await installCompatibilityTool();
    }

    co_return true;
}

QCoro::Task<bool> AssetUpdater::checkRemoteDxvkVersion()
{
    const QNetworkRequest request(dxvkReleasesUrl());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await launcher.mgr()->get(request);
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.assetError(i18n("Could not check for the latest DXVK version.\n\n%1", reply->errorString()));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::assetDecided);
        co_return shouldContinue;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray releases = doc.array();
    const QJsonObject latestRelease = releases.first().toObject();

    if (latestRelease.contains("tag_name"_L1)) {
        m_remoteDxvkToolVersion = latestRelease["tag_name"_L1].toString();
    } else if (latestRelease.contains("name"_L1)) {
        m_remoteDxvkToolVersion = latestRelease["name"_L1].toString();
    } else {
        m_remoteDxvkToolVersion = latestRelease["commit"_L1].toObject()["sha"_L1].toString();
    }
    m_remoteDxvkToolUrl = latestRelease["assets"_L1].toArray().first()["browser_download_url"_L1].toString();

    qInfo(ASTRA_LOG) << "DXVK remote version" << m_remoteDxvkToolVersion;

    QString localDxvkVersion;
    const QString dxvkVer = m_dxvkDir.absoluteFilePath(QStringLiteral("dxvk.ver"));
    if (QFile::exists(dxvkVer)) {
        localDxvkVersion = Utility::readVersion(dxvkVer);
        qInfo(ASTRA_LOG) << "Local DXVK version:" << localDxvkVersion;
    }

    // TODO: this version should not be profile specific
    if (m_remoteDxvkToolVersion != localDxvkVersion) {
        qInfo(ASTRA_LOG) << "DXVK tool out of date";

        co_return co_await installDxvkTool();
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
    query.addQueryItem(QStringLiteral("track"), m_profile.dalamudChannelName());

    url.setQuery(query);

    const QNetworkRequest request(url);
    Utility::printRequest(QStringLiteral("GET"), request);

    m_remoteDalamudVersion.clear();
    m_remoteRuntimeVersion.clear();

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.dalamudError(i18n("Could not check for Dalamud updates.\n\n%1", reply->errorString()));

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

    // TODO: Also check supportedGameVer as a fallback
    m_profile.setDalamudApplicable(doc["isApplicableForCurrentGameVer"_L1].toBool());

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

QCoro::Task<bool> AssetUpdater::installCompatibilityTool() const
{
    Q_EMIT launcher.stageChanged(i18n("Updating compatibility tool..."));

    const auto request = QNetworkRequest(QUrl(m_remoteCompatibilityToolUrl));
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.miscError(i18n("Could not update compatibility tool:\n\n%1", reply->errorString()));
        co_return false;
    }

    qInfo(ASTRA_LOG) << "Finished downloading compatibility tool";

    QFile file(m_tempDir.filePath(QStringLiteral("wine.tar.xz")));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    KTar archive(m_tempDir.filePath(QStringLiteral("wine.tar.xz")));
    if (!archive.open(QIODevice::ReadOnly)) {
        qCritical(ASTRA_LOG) << "Failed to install compatibility tool";
        Q_EMIT launcher.miscError(i18n("Failed to install compatibility tool."));
        co_return false;
    }

    const auto *root = dynamic_cast<const KArchiveDirectory *>(archive.directory()->entry(archive.directory()->entries().first()));
    Q_UNUSED(root->copyTo(m_wineDir.absolutePath(), true))

    archive.close();

    Utility::writeVersion(m_wineDir.absoluteFilePath(QStringLiteral("wine.ver")), m_remoteCompatibilityToolVersion);

    m_profile.setCompatibilityToolVersion(m_remoteCompatibilityToolVersion);

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installDxvkTool() const
{
    Q_EMIT launcher.stageChanged(i18n("Updating DXVK..."));

    const auto request = QNetworkRequest(QUrl(m_remoteDxvkToolUrl));
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await launcher.mgr()->get(request);

    qInfo(ASTRA_LOG) << "Finished downloading DXVK";

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.miscError(i18n("Could not update DXVK:\n\n%1", reply->errorString()));
        co_return false;
    }

    QFile file(m_tempDir.filePath(QStringLiteral("dxvk.tar.xz")));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    KTar archive(m_tempDir.filePath(QStringLiteral("dxvk.tar.xz")));
    if (!archive.open(QIODevice::ReadOnly)) {
        qCritical(ASTRA_LOG) << "Failed to install DXVK";
        Q_EMIT launcher.miscError(i18n("Failed to install DXVK."));
        co_return false;
    }

    const auto *root = dynamic_cast<const KArchiveDirectory *>(archive.directory()->entry(archive.directory()->entries().first()));
    Q_UNUSED(root->copyTo(m_dxvkDir.absolutePath(), true))

    archive.close();

    Utility::writeVersion(m_dxvkDir.absoluteFilePath(QStringLiteral("dxvk.ver")), m_remoteDxvkToolVersion);

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
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    if (!extractZip(m_tempDir.filePath(QStringLiteral("dalamud-assets.zip")), m_dalamudAssetDir.absolutePath())) {
        qCritical(ASTRA_LOG) << "Failed to install Dalamud assets";
        Q_EMIT launcher.dalamudError(i18n("Failed to install Dalamud assets."));

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
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    if (!extractZip(m_tempDir.filePath(QStringLiteral("latest.zip")), m_dalamudDir.absoluteFilePath(m_profile.dalamudChannelName()))) {
        qCritical(ASTRA_LOG) << "Failed to install Dalamud";
        Q_EMIT launcher.dalamudError(i18n("Failed to install Dalamud."));

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

    bool success = extractZip(m_tempDir.filePath(QStringLiteral("dotnet-core.zip")), m_dalamudRuntimeDir.absolutePath());
    success |= extractZip(m_tempDir.filePath(QStringLiteral("dotnet-desktop.zip")), m_dalamudRuntimeDir.absolutePath());

    if (!success) {
        qCritical(ASTRA_LOG) << "Failed to install dotnet";
        Q_EMIT launcher.dalamudError(i18n("Failed to install .NET runtime."));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::dalamudDecided);
        co_return shouldContinue;
    } else {
        Utility::writeVersion(m_dalamudRuntimeDir.absoluteFilePath(QStringLiteral("runtime.ver")), m_remoteRuntimeVersion);

        co_return true;
    }
}

QUrl AssetUpdater::dalamudVersionManifestUrl() const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/VersionInfo"));

    return url;
}

QUrl AssetUpdater::dalamudAssetManifestUrl() const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Asset/Meta"));

    return url;
}

QUrl AssetUpdater::dotnetRuntimePackageUrl(const QString &version) const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/Runtime/DotNet/%1").arg(version));

    return url;
}

QUrl AssetUpdater::dotnetDesktopPackageUrl(const QString &version) const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->dalamudDistribServer());
    url.setPath(QStringLiteral("/Dalamud/Release/Runtime/WindowsDesktop/%1").arg(version));

    return url;
}

QUrl AssetUpdater::dxvkReleasesUrl() const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->githubApi());
    url.setPath(QStringLiteral("/repos/%1/releases").arg(launcher.config()->dXVKRepository()));

    return url;
}

QUrl AssetUpdater::wineReleasesUrl() const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->githubApi());
    url.setPath(QStringLiteral("/repos/%1/releases").arg(launcher.config()->wineRepository()));

    return url;
}

bool AssetUpdater::extractZip(const QString &filePath, const QString &directory)
{
    KZip archive(filePath);
    if (!archive.open(QIODevice::ReadOnly)) {
        return false;
    }

    const KArchiveDirectory *root = archive.directory();
    if (!root->copyTo(directory, true)) {
        archive.close();
        return false;
    }

    archive.close();

    return true;
}

#include "moc_assetupdater.cpp"
