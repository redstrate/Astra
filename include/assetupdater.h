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

    QTemporaryDir tempDir;

    bool doneDownloadingDalamud = true;
    bool doneDownloadingNativelauncher = true;
    bool doneDownloadingRuntimeCore = true;
    bool doneDownloadingRuntimeDesktop = true;
    bool needsRuntimeInstall = false;
    bool needsDalamudInstall = false;
    bool needsNativeInstall = false;

    int remoteDalamudAssetVersion = -1;
    QList<QString> dalamudAssetNeededFilenames;
    QJsonArray remoteDalamudAssetArray;

    QString dataDir;

    bool wantsCancel = false;
};
