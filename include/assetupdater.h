#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QProgressDialog>
#include <QJsonArray>

class LauncherCore;
class QNetworkReply;
struct ProfileSettings;

class AssetUpdater : public QObject {
    Q_OBJECT
public:
    AssetUpdater(LauncherCore& launcher);

    void update(const ProfileSettings& profile);
    void beginInstall();

    void checkIfCheckingIsDone();
    void checkIfDalamudAssetsDone();
    void checkIfFinished();

signals:
    void finishedUpdating();

private:
    LauncherCore& launcher;

    QProgressDialog* dialog;

    const ProfileSettings* currentSettings = nullptr;

    QString remoteDalamudVersion;
    QString remoteRuntimeVersion;
    QString remoteNativeLauncherVersion;

    QTemporaryDir tempDir;

    bool doneDownloadingDalamud = false;
    bool doneDownloadingNativelauncher = false;
    bool doneDownloadingRuntimeCore = false;
    bool doneDownloadingRuntimeDesktop = false;
    bool needsRuntimeInstall = false;
    bool needsDalamudInstall = false;
    bool needsNativeInstall = false;

    int remoteDalamudAssetVersion = -1;
    QList<QString> dalamudAssetNeededFilenames;
    QJsonArray remoteDalamudAssetArray;

    QString dataDir;
};
