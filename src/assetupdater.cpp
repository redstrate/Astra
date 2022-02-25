#include "assetupdater.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QJsonArray>

#include <quazip/JlCompress.h>

#include "launchercore.h"

const QString baseGoatcorpDomain = "https://goatcorp.github.io";

const QString dalamudRemotePath = baseGoatcorpDomain + "/dalamud-distrib";
const QString dalamudVersion = "/latest";
const QString dalamudVersionPath = dalamudRemotePath + "/version";

const QString dalamudAssetRemotePath = baseGoatcorpDomain + "/DalamudAssets";
const QString dalamudAssetManifestPath = dalamudAssetRemotePath + "/asset.json";

const QString nativeLauncherRemotePath =
    "https://github.com/redstrate/nativelauncher/releases/download/";
const QString nativeLauncherVersion = "v1.0.0";

AssetUpdater::AssetUpdater(LauncherCore& launcher) : launcher(launcher) {
    connect(launcher.mgr, &QNetworkAccessManager::finished, this,
            &AssetUpdater::finishDownload);

    launcher.mgr->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

void AssetUpdater::update(const ProfileSettings& profile) {
    // non-dalamud users can bypass this process since it's not needed
    if(!profile.enableDalamud) {
        finishedUpdating();
        return;
    }

    const QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    qInfo() << "Starting update sequence...";

    const bool hasDalamud = QFile::exists(dataDir + "/NativeLauncher.exe") &&
                            QFile::exists(dataDir + "/Dalamud");

    bool isDalamudUpdated = false;
    if (hasDalamud) {
        if (remoteDalamudVersion.isEmpty()) {
            QNetworkRequest request(dalamudVersionPath);

            auto reply = launcher.mgr->get(request);
            reply->setObjectName("DalamudVersionCheck");
            currentSettings = &profile; // TODO: this is dirty, should change

            return;
        } else {
            if (profile.dalamudVersion != remoteDalamudVersion) {
                isDalamudUpdated = false;
            } else {
                qInfo() << "No need to update Dalamud.";
                isDalamudUpdated = true;
            }
        }
    }

   {
        qInfo() << "Checking Dalamud assets...";

        // we want to prevent logging in before we actually check the version
        dalamudAssetNeededFilenames.append("dummy");

        // first we want to fetch the list of assets required
        QNetworkRequest request(dalamudAssetManifestPath);

        auto reply = launcher.mgr->get(request);

        connect(reply, &QNetworkReply::finished, [reply, this, profile] {
            // lol, they actually provide invalid json. let's fix it if it's borked
            QString badJson = reply->readAll();

            qInfo() << reply->errorString();
            qInfo() << "Got asset manifest: " << badJson;

            auto lastCommaLoc = badJson.lastIndexOf(',');
            auto lastBracketLoc = badJson.lastIndexOf('{');

            qInfo() << "Location of last comma: " << lastCommaLoc;
            qInfo() << "Location of last bracket: " << lastBracketLoc;

            // basically, if { supersedes the last ,
            if (lastCommaLoc > lastBracketLoc) {
                qInfo() << "Dalamud server gave bad json, attempting to fix...";
                badJson.remove(lastCommaLoc, 1);
            } else {
                qInfo() << "Got valid json.";
            }

            QJsonDocument doc = QJsonDocument::fromJson(badJson.toUtf8());

            qInfo() << "Dalamud asset remote version" << doc.object()["Version"].toInt();
            qInfo() << "Dalamud asset local version" << profile.dalamudAssetVersion;

            remoteDalamudAssetVersion = doc.object()["Version"].toInt();

            if(remoteDalamudAssetVersion != profile.dalamudAssetVersion) {
                qInfo() << "Dalamud assets out of date.";

                dalamudAssetNeededFilenames.clear();

                for(auto assetObject : doc.object()["Assets"].toArray()) {
                    {
                        qInfo() << "Starting download for " << assetObject.toObject()["FileName"];

                        dalamudAssetNeededFilenames.append(assetObject.toObject()["FileName"].toString());

                        QNetworkRequest assetRequest(assetObject.toObject()["Url"].toString());
                        auto assetReply = launcher.mgr->get(assetRequest);

                        connect(assetReply, &QNetworkReply::finished, [this, assetReply, assetObject = assetObject.toObject()] {
                            const QString dataDir =
                                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/DalamudAssets/";

                            if (!QDir().exists(dataDir))
                                QDir().mkdir(dataDir);

                            const QString fileName = assetObject["FileName"].toString();
                            const QList<QString> dirPath = fileName.left(fileName.lastIndexOf("/")).split('/');

                            qInfo() << "Needed directories: " << dirPath;

                            QString build = dataDir;
                            for(auto dir : dirPath) {
                                if (!QDir().exists(build + dir))
                                    QDir().mkdir(build + dir);

                                build += dir + "/";
                            }

                            QFile file(dataDir + assetObject["FileName"].toString());
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
        });
    }

    // first we determine if we need dalamud
    const bool needsDalamud =
        profile.enableDalamud && (!hasDalamud || !isDalamudUpdated);
    if (needsDalamud) {
        // download nativelauncher release (needed to launch the game with fixed
        // ACLs)
        {
            qInfo() << "Downloading NativeLauncher...";
            doneDownloadingNativelauncher = false;
            needsInstall = true;

            QNetworkRequest request(nativeLauncherRemotePath +
                                    nativeLauncherVersion +
                                    "/NativeLauncher.exe");

            auto reply = launcher.mgr->get(request);
            reply->setObjectName("NativeLauncher");
        }

        // download dalamud (... duh)
        {
            qInfo() << "Downloading Dalamud...";
            doneDownloadingDalamud = false;
            needsInstall = true;

            QNetworkRequest request(dalamudRemotePath + dalamudVersion +
                                    ".zip");

            auto reply = launcher.mgr->get(request);
            reply->setObjectName("Dalamud");
        }
    }
}

void AssetUpdater::finishDownload(QNetworkReply* reply) {
    if (reply->objectName() == "Dalamud") {
        qInfo() << "Dalamud finished downloading!";

        QFile file(tempDir.path() + "/latest.zip");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        doneDownloadingDalamud = true;
        checkIfFinished();
    } else if (reply->objectName() == "NativeLauncher") {
        qInfo() << "NativeLauncher finished downloading!";

        QFile file(tempDir.path() + "/NativeLauncher.exe");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        doneDownloadingNativelauncher = true;
        checkIfFinished();
    } else if (reply->objectName() == "DalamudVersionCheck") {
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

        qInfo() << "Latest Dalamud version reported: " << remoteDalamudVersion;

        update(*currentSettings);
        currentSettings = nullptr;
    }
}

void AssetUpdater::beginInstall() {
    QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    bool success = !JlCompress::extractDir(tempDir.path() + "/latest.zip",
                                           dataDir + "/Dalamud")
                        .empty();
    if (success) {
        QFile::copy(tempDir.path() + "/NativeLauncher.exe",
                    dataDir + "/NativeLauncher.exe");

        finishedUpdating();
    } else {
        // STUB: install failure
    }
}

void AssetUpdater::checkIfDalamudAssetsDone() {
    if(dalamudAssetNeededFilenames.empty()) {
        qInfo() << "Finished downloading Dalamud assets.";

        const QString dataDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/DalamudAssets/";

        QFile file(dataDir + "asset.ver");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(QString::number(remoteDalamudAssetVersion).toUtf8());
        file.close();

        checkIfFinished();
    }
}
void AssetUpdater::checkIfFinished() {
    if (doneDownloadingDalamud &&
        doneDownloadingNativelauncher &&
        dalamudAssetNeededFilenames.empty()) {
        if(needsInstall)
            beginInstall();
        else
            finishedUpdating();
    }
}
