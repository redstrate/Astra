// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "assetupdater.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStandardPaths>
#include <qcorofuture.h>
#include <qcoronetworkreply.h>

#include <JlCompress.h>
#include <QtConcurrentRun>

const QString dotnetRuntimePackageURL = QStringLiteral("https://dotnetcli.azureedge.net/dotnet/Runtime/%1/dotnet-runtime-%1-win-x64.zip");
const QString dotnetDesktopPackageURL = QStringLiteral("https://dotnetcli.azureedge.net/dotnet/WindowsDesktop/%1/windowsdesktop-runtime-%1-win-x64.zip");

AssetUpdater::AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , launcher(launcher)
    , chosenChannel(profile.dalamudChannel())
    , m_profile(profile)
{
    launcher.mgr->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

QCoro::Task<> AssetUpdater::update()
{
    if (!m_profile.dalamudEnabled()) {
        co_return;
    }

    qInfo() << "Starting asset update sequence...";

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

    co_await checkRemoteDalamudAssetVersion();
    co_await checkRemoteDalamudVersion();
}

QCoro::Task<> AssetUpdater::checkRemoteDalamudAssetVersion()
{
    // first we want to fetch the list of assets required
    const QNetworkRequest request(dalamudAssetManifestUrl());

    const auto reply = launcher.mgr->get(request);
    co_await reply;

    // TODO: handle asset failure
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

    remoteDalamudAssetVersion = doc.object()[QLatin1String("Version")].toInt();
    remoteDalamudAssetArray = doc.object()[QLatin1String("Assets")].toArray();

    qInfo() << "Dalamud asset remote version" << remoteDalamudAssetVersion;
    qInfo() << "Dalamud asset local version" << m_profile.dalamudAssetVersion();

    // dalamud assets
    if (remoteDalamudAssetVersion != m_profile.dalamudAssetVersion()) {
        qInfo() << "Dalamud assets out of date.";

        co_await installDalamudAssets();
    }
}

QCoro::Task<> AssetUpdater::checkRemoteDalamudVersion()
{
    const QNetworkRequest request(dalamudVersionManifestUrl(m_profile.dalamudChannel()));

    remoteDalamudVersion.clear();
    remoteRuntimeVersion.clear();

    const auto reply = launcher.mgr->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.loginError(i18n("Could not check for Dalamud updates.\n\n%1", reply->errorString()));
        co_return;
    }

    const QByteArray str = reply->readAll();

    // for some god forsaken reason, the version string comes back as raw
    // bytes, ex: \xFF\xFE{\x00\"\x00""A\x00s\x00s\x00""e\x00m\x00 so we
    // start at the first character of the json '{' and work our way up.
    QString reassmbled;
    for (int i = static_cast<int>(str.indexOf('{')); i < str.size(); i++) {
        char t = str[i];
        if (QChar(t).isPrint())
            reassmbled += t;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reassmbled.toUtf8());
    remoteDalamudVersion = doc[QLatin1String("AssemblyVersion")].toString();
    remoteRuntimeVersion = doc[QLatin1String("RuntimeVersion")].toString();

    qInfo() << "Latest Dalamud version reported:" << remoteDalamudVersion << "local:" << m_profile.dalamudVersion();
    qInfo() << "Latest NET runtime reported:" << remoteRuntimeVersion;

    if (remoteDalamudVersion != m_profile.dalamudVersion()) {
        qInfo() << "Downloading Dalamud...";

        co_await installDalamud();
    }

    if (m_profile.runtimeVersion() != remoteRuntimeVersion) {
        qInfo() << "Downloading Runtime...";

        co_await installRuntime();
    }
}

QCoro::Task<> AssetUpdater::installDalamudAssets()
{
    Q_EMIT launcher.stageChanged(i18n("Updating Dalamud assets..."));

    QFutureSynchronizer<void> synchronizer;

    for (const auto &assetObject : remoteDalamudAssetArray) {
        const QNetworkRequest assetRequest(assetObject.toObject()[QLatin1String("Url")].toString());
        const auto assetReply = launcher.mgr->get(assetRequest);

        const auto future = QtFuture::connect(assetReply, &QNetworkReply::finished).then([this, assetReply, assetObject] {
            const QString fileName = assetObject.toObject()[QLatin1String("FileName")].toString();
            const QString dirPath = fileName.left(fileName.lastIndexOf(QLatin1Char('/')));

            const QString path = dalamudAssetDir.absoluteFilePath(dirPath);

            if (!QDir().exists(path))
                QDir().mkpath(path);

            QFile file(dalamudAssetDir.absoluteFilePath(assetObject.toObject()[QLatin1String("FileName")].toString()));
            file.open(QIODevice::WriteOnly);
            file.write(assetReply->readAll());
            file.close();
        });

        synchronizer.addFuture(future);
    }

    co_await QtConcurrent::run([&synchronizer] {
        synchronizer.waitForFinished();
    });

    qInfo() << "Finished downloading Dalamud assets.";

    m_profile.setDalamudAssetVersion(remoteDalamudAssetVersion);

    QFile file(dalamudAssetDir.absoluteFilePath(QStringLiteral("asset.ver")));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(QString::number(remoteDalamudAssetVersion).toUtf8());
    file.close();
}

QCoro::Task<> AssetUpdater::installDalamud()
{
    Q_EMIT launcher.stageChanged(i18n("Updating Dalamud..."));

    const QNetworkRequest request(dalamudLatestPackageUrl(chosenChannel));

    const auto reply = launcher.mgr->get(request);
    co_await reply;

    qInfo() << "Dalamud finished downloading!";

    QFile file(tempDir.path() + QStringLiteral("/latest.zip"));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    const bool success =
        !JlCompress::extractDir(tempDir.path() + QLatin1String("/latest.zip"), dalamudDir.absoluteFilePath(m_profile.dalamudChannelName())).empty();

    if (!success) {
        // TODO: handle failure here
        qInfo() << "Failed to install Dalamud!";
    }

    m_profile.setDalamudVersion(remoteDalamudVersion);
}

QCoro::Task<> AssetUpdater::installRuntime()
{
    Q_EMIT launcher.stageChanged(i18n("Updating .NET Runtime..."));

    // core
    {
        const QNetworkRequest request(dotnetRuntimePackageURL.arg(remoteRuntimeVersion));

        const auto reply = launcher.mgr->get(request);
        co_await reply;

        qInfo() << "Dotnet-core finished downloading!";

        QFile file(tempDir.path() + QStringLiteral("/dotnet-core.zip"));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }

    // desktop
    {
        const QNetworkRequest request(dotnetDesktopPackageURL.arg(remoteRuntimeVersion));

        const auto reply = launcher.mgr->get(request);
        co_await reply;

        qInfo() << "Dotnet-desktop finished downloading!";

        QFile file(tempDir.path() + QStringLiteral("/dotnet-desktop.zip"));
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
    }

    bool success = !JlCompress::extractDir(tempDir.path() + QStringLiteral("/dotnet-core.zip"), dalamudRuntimeDir.absolutePath()).empty();
    success |= !JlCompress::extractDir(tempDir.path() + QStringLiteral("/dotnet-desktop.zip"), dalamudRuntimeDir.absolutePath()).empty();

    if (!success) {
        qInfo() << "Failed to install dotnet!";
    } else {
        QFile file(dalamudRuntimeDir.absoluteFilePath(QStringLiteral("runtime.ver")));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(remoteRuntimeVersion.toUtf8());
        file.close();
    }
}

static const QMap<Profile::DalamudChannel, QString> channelToDistribPrefix = {{Profile::DalamudChannel::Stable, QStringLiteral("/")},
                                                                              {Profile::DalamudChannel::Staging, QStringLiteral("stg/")},
                                                                              {Profile::DalamudChannel::Net5, QStringLiteral("net5/")}};

QUrl AssetUpdater::dalamudVersionManifestUrl(const Profile::DalamudChannel channel) const
{
    QUrl url;
    url.setScheme(launcher.preferredProtocol());
    url.setHost(launcher.dalamudDistribServer());
    url.setPath(QStringLiteral("/dalamud-distrib/%1version").arg(channelToDistribPrefix[channel]));

    return url;
}

QUrl AssetUpdater::dalamudLatestPackageUrl(Profile::DalamudChannel channel) const
{
    QUrl url;
    url.setScheme(launcher.preferredProtocol());
    url.setHost(launcher.dalamudDistribServer());
    url.setPath(QStringLiteral("/dalamud-distrib/%1latest.zip").arg(channelToDistribPrefix[channel]));

    return url;
}

QUrl AssetUpdater::dalamudAssetManifestUrl() const
{
    QUrl url;
    url.setScheme(launcher.preferredProtocol());
    url.setHost(launcher.dalamudDistribServer());
    url.setPath(QStringLiteral("/DalamudAssets/asset.json"));

    return url;
}
