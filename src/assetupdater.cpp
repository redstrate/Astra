#include "assetupdater.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>

#include <quazip/JlCompress.h>

#include "launchercore.h"

const QString dalamudRemotePath = "https://goatcorp.github.io/dalamud-distrib/";
const QString dalamudVersion = "latest";
const QString dalamudVersionPath = dalamudRemotePath + "version";

const QString nativeLauncherRemotePath =
    "https://github.com/redstrate/nativelauncher/releases/download/";
const QString nativeLauncherVersion = "v1.0.0";

AssetUpdater::AssetUpdater(LauncherCore& launcher) : launcher(launcher) {
    connect(launcher.mgr, &QNetworkAccessManager::finished, this,
            &AssetUpdater::finishDownload);

    launcher.mgr->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

void AssetUpdater::update(const ProfileSettings& profile) {
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
            if (QFile::exists(dataDir + "/Dalamud/Dalamud.deps.json")) {
                QFile depsJson(dataDir + "/Dalamud/Dalamud.deps.json");
                depsJson.open(QFile::ReadOnly);
                QJsonDocument doc = QJsonDocument::fromJson(depsJson.readAll());

                // TODO: UGLY
                QString versionString =
                    doc["targets"]
                        .toObject()[".NETCoreApp,Version=v5.0"]
                        .toObject()
                        .keys()
                        .filter("Dalamud")[0];
                versionString = versionString.remove("Dalamud/");

                qInfo() << "Dalamud version installed: " << versionString;

                if (versionString != remoteDalamudVersion) {
                    isDalamudUpdated = false;
                } else {
                    qInfo() << "No need to update Dalamud.";
                    isDalamudUpdated = true;
                }
            }
        }
    }

    // first we determine if we need dalamud
    const bool needsDalamud =
        profile.enableDalamud && (!hasDalamud || !isDalamudUpdated);
    if (needsDalamud) {
        // download nativelauncher release (needed to launch the game with fixed
        // ACLs)
        {
            qInfo() << "Downloading NativeLauncher...";

            QNetworkRequest request(nativeLauncherRemotePath +
                                    nativeLauncherVersion +
                                    "/NativeLauncher.exe");

            auto reply = launcher.mgr->get(request);
            reply->setObjectName("NativeLauncher");
        }

        // download dalamud (... duh)
        {
            qInfo() << "Downloading Dalamud...";

            QNetworkRequest request(dalamudRemotePath + dalamudVersion +
                                    ".zip");

            auto reply = launcher.mgr->get(request);
            reply->setObjectName("Dalamud");
        }
    } else {
        // non-dalamud users can bypass this process since it's not needed
        finishedUpdating();
    }
}

void AssetUpdater::finishDownload(QNetworkReply* reply) {
    const auto checkIfFinished = [=] {
        if (QFile::exists(tempDir.path() + "/NativeLauncher.exe") &&
            QFile::exists(tempDir.path() + "/latest.zip")) {
            beginInstall();
        }
    };

    if (reply->objectName() == "Dalamud") {
        qInfo() << "Dalamud finished downloading!";

        QFile file(tempDir.path() + "/latest.zip");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        checkIfFinished();
    } else if (reply->objectName() == "NativeLauncher") {
        qInfo() << "NativeLauncher finished downloading!";

        QFile file(tempDir.path() + "/NativeLauncher.exe");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

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
