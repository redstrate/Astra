#pragma once

#include <QObject>
#include <QTemporaryDir>

class LauncherCore;
class QNetworkReply;
struct ProfileSettings;

class AssetUpdater : public QObject {
    Q_OBJECT
public:
    AssetUpdater(LauncherCore& launcher);

    void update(const ProfileSettings& profile);
    void finishDownload(QNetworkReply* reply);
    void beginInstall();

    void checkIfDalamudAssetsDone();
    void checkIfFinished();

signals:
    void finishedUpdating();

private:
    LauncherCore& launcher;

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

    int remoteDalamudAssetVersion;
    QList<QString> dalamudAssetNeededFilenames;
};
