#pragma once

#include <QNetworkAccessManager>
#include <QString>
#include <physis.hpp>

// General-purpose patcher routine. It opens a nice dialog box, handles downloading
// and processing patches.
class Patcher : public QObject
{
    Q_OBJECT
public:
    Patcher(QString baseDirectory, GameData *game_data, QObject *parent = nullptr);
    Patcher(QString baseDirectory, BootData *game_data, QObject *parent = nullptr);

    void processPatchList(QNetworkAccessManager &mgr, const QString &patchList);

signals:
    void done();

private:
    void checkIfDone();

    [[nodiscard]] bool isBoot() const
    {
        return boot_data != nullptr;
    }

    struct QueuedPatch {
        QString name, repository, version, path;
    };

    void processPatch(const QueuedPatch &patch);

    QVector<QueuedPatch> patchQueue;

    QString baseDirectory;
    BootData *boot_data = nullptr;
    GameData *game_data = nullptr;

    int remainingPatches = -1;
};