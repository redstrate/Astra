// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "patchlist.h"
#include <QDir>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QString>
#include <physis.hpp>
#include <qcorotask.h>

class LauncherCore;

// General-purpose patcher routine. It opens a nice dialog box, handles downloading
// and processing patches.
class Patcher : public QObject
{
    Q_OBJECT

public:
    Patcher(LauncherCore &launcher, const QString &baseDirectory, GameData &gameData, QObject *parent = nullptr);
    Patcher(LauncherCore &launcher, const QString &baseDirectory, BootData &bootData, QObject *parent = nullptr);
    ~Patcher() override;

    QCoro::Task<bool> patch(const PatchList &patchList);

private:
    void setupDirectories();
    [[nodiscard]] QString getBaseString() const;

    [[nodiscard]] bool isBoot() const
    {
        return m_bootData != nullptr;
    }

    struct QueuedPatch {
        QString name, repository, version, path;
        QStringList hashes;
        long hashBlockSize;
        long length;
        bool isBoot;

        long bytesDownloaded;
        bool downloaded = false;

        [[nodiscard]] QString getVersion() const
        {
            if (isBoot) {
                return QStringLiteral("ffxivboot - %1").arg(name);
            } else {
                return QStringLiteral("%1 - %2").arg(repository, name);
            }
        }
    };

    void processPatch(const QueuedPatch &patch);

    QVector<QueuedPatch> m_patchQueue;

    QDir m_patchesDir;
    QString m_baseDirectory;
    BootData *m_bootData = nullptr;
    GameData *m_gameData = nullptr;

    int m_remainingPatches = -1;

    LauncherCore &m_launcher;

    QMutex m_finishedPatchesMutex;
    int m_finishedPatches = 0;

    void updateDownloadProgress(int index, int received);
    void updateMessage();
};
