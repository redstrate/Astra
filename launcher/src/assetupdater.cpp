#include "assetupdater.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStandardPaths>

#include <JlCompress.h>

const QString baseGoatDomain = "https://goatcorp.github.io";

const QString baseDalamudDistribution = baseGoatDomain + "/dalamud-distrib/";
const QString dalamudLatestPackageURL = baseDalamudDistribution + "%1latest.zip";
const QString dalamudVersionManifestURL = baseDalamudDistribution + "%1version";

const QString baseDalamudAssetDistribution = baseGoatDomain + "/DalamudAssets";
const QString dalamudAssetManifestURL = baseDalamudAssetDistribution + "/asset.json";

const QString dotnetRuntimePackageURL = "https://dotnetcli.azureedge.net/dotnet/Runtime/%1/dotnet-runtime-%1-win-x64.zip";
const QString dotnetDesktopPackageURL = "https://dotnetcli.azureedge.net/dotnet/WindowsDesktop/%1/windowsdesktop-runtime-%1-win-x64.zip";

QMap<Profile::DalamudChannel, QString> channelToDistribPrefix = {{Profile::DalamudChannel::Stable, "/"},
                                                                 {Profile::DalamudChannel::Staging, "stg/"},
                                                                 {Profile::DalamudChannel::Net5, "net5/"}};

AssetUpdater::AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , launcher(launcher)
    , m_profile(profile)
{
    launcher.mgr->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (!QDir().exists(dataDir))
        QDir().mkdir(dataDir);
}

void AssetUpdater::update()
{
    // non-dalamud users can bypass this process since it's not needed
    if (!m_profile.dalamudEnabled()) {
        Q_EMIT finishedUpdating();
        return;
    }

    Q_EMIT launcher.stageChanged("Updating assets...");

    // first, we want to collect all of the remote versions

    qInfo() << "Starting update sequence...";
    // dialog->setLabelText("Checking for updates...");

    // dalamud assets
    {
        // we want to prevent logging in before we actually check the version
        dalamudAssetNeededFilenames.clear();
        remoteDalamudAssetVersion = -1;

        dalamudAssetNeededFilenames.append("dummy");

        // first we want to fetch the list of assets required
        QNetworkRequest request(dalamudAssetManifestURL);

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [reply, this] {
            Q_EMIT launcher.stageChanged("Checking for Dalamud asset updates...");

            // TODO: handle asset failure
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

            qInfo() << "Dalamud asset remote version" << doc.object()["Version"].toInt();
            qInfo() << "Dalamud asset local version" << m_profile.dalamudAssetVersion;

            remoteDalamudAssetVersion = doc.object()["Version"].toInt();

            remoteDalamudAssetArray = doc.object()["Assets"].toArray();

            checkIfCheckingIsDone();
        });
    }

    // dalamud injector / net runtime
    // they're all updated in unison, so there's no reason to have multiple checks
    {
        QNetworkRequest request(dalamudVersionManifestURL.arg(channelToDistribPrefix[m_profile.dalamudChannel()]));

        chosenChannel = m_profile.dalamudChannel();

        remoteDalamudVersion.clear();
        remoteRuntimeVersion.clear();

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, reply] {
            Q_EMIT launcher.stageChanged("Checking for Dalamud updates...");

            QByteArray str = reply->readAll();
            // for some god forsaken reason, the version string comes back as raw
            // bytes, ex: \xFF\xFE{\x00\"\x00""A\x00s\x00s\x00""e\x00m\x00 so we
            // start at the first character of the json '{' and work our way up.
            QString reassmbled;
            for (int i = str.indexOf('{'); i < str.size(); i++) {
                char t = str[i];
                if (QChar(t).isPrint())
                    reassmbled += t;
            }

            QJsonDocument doc = QJsonDocument::fromJson(reassmbled.toUtf8());
            remoteDalamudVersion = doc["AssemblyVersion"].toString();
            remoteRuntimeVersion = doc["RuntimeVersion"].toString();

            qInfo() << "Latest Dalamud version reported: " << remoteDalamudVersion;
            qInfo() << "Latest NET runtime reported: " << remoteRuntimeVersion;

            checkIfCheckingIsDone();
        });
    }
}

void AssetUpdater::beginInstall()
{
    if (needsDalamudInstall) {
        bool success = !JlCompress::extractDir(tempDir.path() + "/latest.zip", dataDir + "/Dalamud").empty();

        if (!success) {
            // TODO: handle failure here
            qInfo() << "Failed to install Dalamud!";
        } else {
            needsDalamudInstall = false;
        }
    }

    if (needsRuntimeInstall) {
        bool success = !JlCompress::extractDir(tempDir.path() + "/dotnet-core.zip", dataDir + "/DalamudRuntime").empty();

        success |= !JlCompress::extractDir(tempDir.path() + "/dotnet-desktop.zip", dataDir + "/DalamudRuntime").empty();

        if (!success) {
            qInfo() << "Failed to install dotnet!";
        } else {
            QFile file(dataDir + "/DalamudRuntime/runtime.ver");
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            file.write(remoteRuntimeVersion.toUtf8());
            file.close();

            needsRuntimeInstall = false;
        }
    }

    checkIfFinished();
}

void AssetUpdater::checkIfDalamudAssetsDone()
{
    if (dalamudAssetNeededFilenames.empty()) {
        qInfo() << "Finished downloading Dalamud assets.";

        m_profile.dalamudAssetVersion = remoteDalamudAssetVersion;

        QFile file(dataDir + "/DalamudAssets/" + "asset.ver");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(QString::number(remoteDalamudAssetVersion).toUtf8());
        file.close();

        checkIfFinished();
    }
}

void AssetUpdater::checkIfFinished()
{
    if (doneDownloadingDalamud && doneDownloadingRuntimeCore && doneDownloadingRuntimeDesktop && dalamudAssetNeededFilenames.empty()) {
        if (needsRuntimeInstall || needsDalamudInstall) {
            beginInstall();
        } else {
            Q_EMIT finishedUpdating();
        }
    }
}

void AssetUpdater::checkIfCheckingIsDone()
{
    if (remoteDalamudVersion.isEmpty() || remoteRuntimeVersion.isEmpty() || remoteDalamudAssetVersion == -1) {
        return;
    }

    // now that we got all the information we need, let's check if anything is
    // updateable

    Q_EMIT launcher.stageChanged("Starting Dalamud update...");

    // dalamud injector / net runtime
    if (m_profile.runtimeVersion != remoteRuntimeVersion) {
        needsRuntimeInstall = true;

        // core
        {
            QNetworkRequest request(dotnetRuntimePackageURL.arg(remoteRuntimeVersion));

            auto reply = launcher.mgr->get(request);
            connect(reply, &QNetworkReply::finished, [this, reply] {
                qInfo() << "Dotnet-core finished downloading!";

                Q_EMIT launcher.stageChanged("Updating Dotnet-core...");

                QFile file(tempDir.path() + "/dotnet-core.zip");
                file.open(QIODevice::WriteOnly);
                file.write(reply->readAll());
                file.close();

                doneDownloadingRuntimeCore = true;

                checkIfFinished();
            });
        }

        // desktop
        {
            QNetworkRequest request(dotnetDesktopPackageURL.arg(remoteRuntimeVersion));

            auto reply = launcher.mgr->get(request);
            connect(reply, &QNetworkReply::finished, [this, reply] {
                qInfo() << "Dotnet-desktop finished downloading!";

                Q_EMIT launcher.stageChanged("Updating Dotnet-desktop...");

                QFile file(tempDir.path() + "/dotnet-desktop.zip");
                file.open(QIODevice::WriteOnly);
                file.write(reply->readAll());
                file.close();

                doneDownloadingRuntimeDesktop = true;

                checkIfFinished();
            });
        }
    } else {
        doneDownloadingRuntimeCore = true;
        doneDownloadingRuntimeDesktop = true;
        needsRuntimeInstall = false;

        checkIfFinished();
    }

    if (remoteDalamudVersion != m_profile.dalamudVersion) {
        qInfo() << "Downloading Dalamud...";

        needsDalamudInstall = true;

        QNetworkRequest request(dalamudLatestPackageURL.arg(channelToDistribPrefix[chosenChannel]));

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, reply] {
            qInfo() << "Dalamud finished downloading!";

            Q_EMIT launcher.stageChanged("Updating Dalamud...");

            QFile file(tempDir.path() + "/latest.zip");
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.close();

            doneDownloadingDalamud = true;

            m_profile.dalamudVersion = remoteDalamudVersion;

            checkIfFinished();
        });
    } else {
        qInfo() << "No need to update Dalamud.";

        doneDownloadingDalamud = true;
        needsDalamudInstall = false;

        checkIfFinished();
    }

    // dalamud assets
    if (remoteDalamudAssetVersion != m_profile.dalamudAssetVersion) {
        qInfo() << "Dalamud assets out of date.";

        Q_EMIT launcher.stageChanged("Updating Dalamud assets...");

        dalamudAssetNeededFilenames.clear();

        for (auto assetObject : remoteDalamudAssetArray) {
            {
                dalamudAssetNeededFilenames.append(assetObject.toObject()["FileName"].toString());

                QNetworkRequest assetRequest(assetObject.toObject()["Url"].toString());
                auto assetReply = launcher.mgr->get(assetRequest);

                connect(assetReply, &QNetworkReply::finished, [this, assetReply, assetObject = assetObject.toObject()] {
                    if (!QDir().exists(dataDir + "/DalamudAssets"))
                        QDir().mkdir(dataDir + "/DalamudAssets");

                    const QString fileName = assetObject["FileName"].toString();
                    const QList<QString> dirPath = fileName.left(fileName.lastIndexOf("/")).split('/');

                    QString build = dataDir + "/DalamudAssets/";
                    for (const auto &dir : dirPath) {
                        if (!QDir().exists(build + dir))
                            QDir().mkdir(build + dir);

                        build += dir + "/";
                    }

                    QFile file(dataDir + "/DalamudAssets/" + assetObject["FileName"].toString());
                    file.open(QIODevice::WriteOnly);
                    file.write(assetReply->readAll());
                    file.close();

                    dalamudAssetNeededFilenames.removeOne(assetObject["FileName"].toString());
                    checkIfDalamudAssetsDone();
                });
            }
        }
    } else {
        dalamudAssetNeededFilenames.clear();

        qInfo() << "Dalamud assets up to date.";

        checkIfFinished();
    }
}