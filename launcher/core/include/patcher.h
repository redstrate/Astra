#pragma once

#include <QString>
#include <QProgressDialog>
#include <QNetworkAccessManager>

// General-purpose patcher routine. It opens a nice dialog box, handles downloading
// and processing patches.
class Patcher : public QObject {
    Q_OBJECT
public:
    // isBoot is used for telling the patcher that you're reading boot patches, which for some reason has a different patchlist format.
    Patcher(bool isBoot, QString baseDirectory);

    void processPatchList(QNetworkAccessManager& mgr, QString patchList);

signals:
    void done();

private:
    void checkIfDone();

    struct QueuedPatch {
        QString name, repository, version, path;
    };

    void processPatch(QueuedPatch patch);

    QVector<QueuedPatch> patchQueue;

    bool isBoot = false;
    QString baseDirectory;

    QProgressDialog* dialog = nullptr;

    int remainingPatches = -1;
};