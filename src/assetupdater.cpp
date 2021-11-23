#include "assetupdater.h"

#include <QNetworkReply>
#include <QFile>
#include <QStandardPaths>

#include <quazip/JlCompress.h>

#include "launchercore.h"

const QString dalamudRemotePath = "https://goatcorp.github.io/dalamud-distrib/";
const QString dalamudVersion = "latest";

const QString nativeLauncherRemotePath = "https://github.com/redstrate/nativelauncher/releases/download/";
const QString nativeLauncherVersion = "v1.0.0";

AssetUpdater::AssetUpdater(LauncherCore &launcher) : launcher(launcher) {
    connect(launcher.mgr, &QNetworkAccessManager::finished, this, &AssetUpdater::finishDownload);

    launcher.mgr->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

void AssetUpdater::update(const ProfileSettings& profile) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const bool hasDalamud = QFile::exists(dataDir + "/NativeLauncher.exe") && QFile::exists(dataDir + "/Dalamud");

    // first we determine if we need dalamud
    const bool needsDalamud = profile.enableDalamud && !hasDalamud;
    if(needsDalamud) {
        // download nativelauncher release (needed to launch the game with fixed ACLs)
        {
            QNetworkRequest request(nativeLauncherRemotePath + nativeLauncherVersion + "/NativeLauncher.exe");

            auto reply = launcher.mgr->get(request);
            reply->setObjectName("NativeLauncher");
        }

        // download dalamud (... duh)
        {
            QNetworkRequest request(dalamudRemotePath + dalamudVersion + ".zip");

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
        if(QFile::exists(tempDir.path() + "/NativeLauncher.exe") && QFile::exists(tempDir.path() + "/latest.zip")) {
            beginInstall();
        }
    };

    if(reply->objectName() == "Dalamud") {
        QFile file(tempDir.path() +  "/latest.zip");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        checkIfFinished();
    } else if(reply->objectName() == "NativeLauncher") {
        QFile file(tempDir.path() + "/NativeLauncher.exe");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        checkIfFinished();
    }
}

void AssetUpdater::beginInstall() {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    bool success = !JlCompress::extractDir(tempDir.path() + "/latest.zip", dataDir + "/Dalamud").empty();
    if(success) {
        QFile::copy(tempDir.path() + "/NativeLauncher.exe", dataDir + "/NativeLauncher.exe");

        finishedUpdating();
    } else {
        // STUB: install failure
    }
}