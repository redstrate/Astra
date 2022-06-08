#include "assetupdater.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QJsonArray>

#include <JlCompress.h>

#include "launchercore.h"

const QString baseGoatDomain = "https://goatcorp.github.io";

const QString baseDalamudDistribution = baseGoatDomain + "/dalamud-distrib/";
const QString dalamudLatestPackageURL = baseDalamudDistribution + "%1latest.zip";
const QString dalamudVersionManifestURL = baseDalamudDistribution + "%1version";

const QString baseDalamudAssetDistribution = baseGoatDomain + "/DalamudAssets";
const QString dalamudAssetManifestURL = baseDalamudAssetDistribution + "/asset.json";

const QString baseNativeLauncherDistribution =
    "https://xiv.zone/astra-distrib/nativelauncher";
const QString nativeLauncherLatestPackageURL =
    baseNativeLauncherDistribution + "/NativeLauncher.exe";
const QString nativeLauncherVersionManifestURL = baseNativeLauncherDistribution + "/version";

const QString dotnetRuntimePackageURL =
    "https://dotnetcli.azureedge.net/dotnet/Runtime/%1/dotnet-runtime-%1-win-x64.zip";
const QString dotnetDesktopPackageURL =
    "https://dotnetcli.azureedge.net/dotnet/WindowsDesktop/%1/windowsdesktop-runtime-%1-win-x64.zip";

QMap<DalamudChannel, QString> channelToDistribPrefix = {
    {DalamudChannel::Stable, "/"},
    {DalamudChannel::Staging, "stg/"},
    {DalamudChannel::Net5, "net5/"}
};

AssetUpdater::AssetUpdater(LauncherCore& launcher) : launcher(launcher) {
    launcher.mgr->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if(!QDir().exists(dataDir))
        QDir().mkdir(dataDir);
}

void AssetUpdater::update(const ProfileSettings& profile) {
    // non-dalamud users can bypass this process since it's not needed
    if(!profile.dalamud.enabled) {
        finishedUpdating();
        return;
    }

    dialog = new QProgressDialog("Updating assets...", "Cancel", 0, 0);

    // first, we want to collect all of the remote versions

    qInfo() << "Starting update sequence...";
    dialog->setLabelText("Checking for updates...");

    // dalamud assets
    {
        // we want to prevent logging in before we actually check the version
        dalamudAssetNeededFilenames.clear();
        remoteDalamudAssetVersion = -1;

        dalamudAssetNeededFilenames.append("dummy");

        // first we want to fetch the list of assets required
        QNetworkRequest request(dalamudAssetManifestURL);

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [reply, this, profile] {
            dialog->setLabelText("Checking for Dalamud asset updates...");

            // TODO: handle asset failure
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

            qInfo() << "Dalamud asset remote version" << doc.object()["Version"].toInt();
            qInfo() << "Dalamud asset local version" << launcher.dalamudAssetVersion;

            remoteDalamudAssetVersion = doc.object()["Version"].toInt();

            remoteDalamudAssetArray = doc.object()["Assets"].toArray();

            checkIfCheckingIsDone();
        });
    }

    // native launcher
    {
        QNetworkRequest request(nativeLauncherVersionManifestURL);

        remoteNativeLauncherVersion.clear();

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, profile, reply] {
            dialog->setLabelText("Checking for native launcher updates...");

            remoteNativeLauncherVersion = reply->readAll().trimmed();

            qInfo() << "Latest native launcher version reported: " << remoteNativeLauncherVersion;

            checkIfCheckingIsDone();
        });
    }

    // dalamud injector / net runtime / nativelauncher
    // they're all updated in unison, so there's no reason to have multiple checks
    {
        QNetworkRequest request(dalamudVersionManifestURL.arg(channelToDistribPrefix[profile.dalamud.channel]));

        chosenChannel = profile.dalamud.channel;

        remoteDalamudVersion.clear();
        remoteRuntimeVersion.clear();

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, profile, reply] {
            dialog->setLabelText("Checking for Dalamud updates...");

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

void AssetUpdater::beginInstall() {
    if(needsDalamudInstall) {
        bool success = !JlCompress::extractDir(tempDir.path() + "/latest.zip",
                                               dataDir + "/Dalamud")
                            .empty();

        if(!success) {
            // TODO: handle failure here
            qInfo() << "Failed to install Dalamud!";
        } else {
            needsDalamudInstall = false;
        }
    }

    if(needsNativeInstall) {
        qInfo() << "Installing native launcher...";

        if(QFile::exists(dataDir + "/NativeLauncher.exe"))
            QFile::remove(dataDir + "/NativeLauncher.exe");

        bool success = QFile::copy(tempDir.path() + "/NativeLauncher.exe",
                    dataDir + "/NativeLauncher.exe");

        if(!success) {
            qInfo() << "Failed to install native launcher!";
        } else {
            QFile file(dataDir + "/nativelauncher.ver");
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            file.write(remoteNativeLauncherVersion.toUtf8());
            file.close();

            needsNativeInstall = false;
        }
    }

    if(needsRuntimeInstall) {
        bool success = !JlCompress::extractDir(tempDir.path() + "/dotnet-core.zip",
                                               dataDir + "/DalamudRuntime")
                            .empty();

        success |= !JlCompress::extractDir(tempDir.path() + "/dotnet-desktop.zip",
                                           dataDir + "/DalamudRuntime")
                        .empty();

        if(!success) {
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

void AssetUpdater::checkIfDalamudAssetsDone() {
    if(dialog->wasCanceled())
        return;

    if(dalamudAssetNeededFilenames.empty()) {
        qInfo() << "Finished downloading Dalamud assets.";

        launcher.dalamudAssetVersion = remoteDalamudAssetVersion;

        QFile file(dataDir + "/DalamudAssets/" + "asset.ver");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(QString::number(remoteDalamudAssetVersion).toUtf8());
        file.close();

        checkIfFinished();
    }
}

void AssetUpdater::checkIfFinished() {
    if(dialog->wasCanceled())
        return;

    if (doneDownloadingDalamud &&
        doneDownloadingNativelauncher &&
        doneDownloadingRuntimeCore &&
        doneDownloadingRuntimeDesktop &&
        dalamudAssetNeededFilenames.empty()) {
        if (needsRuntimeInstall || needsNativeInstall || needsDalamudInstall) {
            beginInstall();
        } else {
            dialog->setLabelText("Finished!");
            dialog->close();

            finishedUpdating();
        }
    }
}

void AssetUpdater::checkIfCheckingIsDone() {
    if(dialog->wasCanceled())
        return;

    if(remoteDalamudVersion.isEmpty() || remoteRuntimeVersion.isEmpty() || remoteDalamudAssetVersion == -1 || remoteNativeLauncherVersion.isEmpty()) {
        return;
    }

    // now that we got all the information we need, let's check if anything is
    // updateable

    dialog->setLabelText("Starting update...");

    // dalamud injector / net runtime
    if(launcher.runtimeVersion != remoteRuntimeVersion) {
        needsRuntimeInstall = true;

        // core
        {
            QNetworkRequest request(dotnetRuntimePackageURL.arg(remoteRuntimeVersion));

            auto reply = launcher.mgr->get(request);
            connect(reply, &QNetworkReply::finished, [this, reply] {
                qInfo() << "Dotnet-core finished downloading!";

                dialog->setLabelText("Updating Dotnet-core...");

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

                dialog->setLabelText("Updating Dotnet-desktop...");

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

    if(remoteDalamudVersion != launcher.dalamudVersion) {
        qInfo() << "Downloading Dalamud...";

        needsDalamudInstall = true;

        QNetworkRequest request(dalamudLatestPackageURL.arg(channelToDistribPrefix[chosenChannel]));

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, reply] {
            qInfo() << "Dalamud finished downloading!";

            dialog->setLabelText("Updating Dalamud...");

            QFile file(tempDir.path() + "/latest.zip");
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.close();

            doneDownloadingDalamud = true;

            launcher.dalamudVersion = remoteDalamudVersion;

            checkIfFinished();
        });
    } else {
        qInfo() << "No need to update Dalamud.";

        doneDownloadingDalamud = true;
        needsDalamudInstall = false;

        checkIfFinished();
    }

    // dalamud assets
    if(remoteDalamudAssetVersion != launcher.dalamudAssetVersion) {
        qInfo() << "Dalamud assets out of date.";

        dialog->setLabelText("Updating Dalamud assets...");

        dalamudAssetNeededFilenames.clear();

        for(auto assetObject : remoteDalamudAssetArray) {
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
                    for(auto dir : dirPath) {
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

    if(remoteNativeLauncherVersion != launcher.nativeLauncherVersion) {
        qInfo() << "Native launcher out of date.";

        dialog->setLabelText("Updating native launcher...");

        needsNativeInstall = true;

        QNetworkRequest request(nativeLauncherLatestPackageURL);

        auto reply = launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, reply] {
            qInfo() << "NativeLauncher finished downloading!";

            QFile file(tempDir.path() + "/NativeLauncher.exe");
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.close();

            doneDownloadingNativelauncher = true;

            checkIfFinished();
        });
    } else {
        qInfo() << "Native launcher up to date.";

        doneDownloadingNativelauncher = true;

        checkIfFinished();
    }
}
