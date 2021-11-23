#pragma once

#include <QObject>
#include <QTemporaryDir>

class LauncherWindow;
class QNetworkReply;

class AssetUpdater : public QObject {
    Q_OBJECT
public:
    AssetUpdater(LauncherWindow& launcher);

    void update();
    void finishDownload(QNetworkReply* reply);
    void beginInstall();

signals:
    void finishedUpdating();

private:
    LauncherWindow& launcher;

    QTemporaryDir tempDir;
};