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

signals:
    void finishedUpdating();

private:
    LauncherCore& launcher;

    QTemporaryDir tempDir;
};