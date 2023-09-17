// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
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
    Patcher(LauncherCore &launcher, QString baseDirectory, GameData *game_data, QObject *parent = nullptr);
    Patcher(LauncherCore &launcher, QString baseDirectory, BootData *game_data, QObject *parent = nullptr);

    QCoro::Task<> patch(QNetworkAccessManager &mgr, const QString &patchList);

private:
    void setupDirectories();

    [[nodiscard]] bool isBoot() const
    {
        return boot_data != nullptr;
    }

    struct QueuedPatch {
        QString name, repository, version, path;
        QStringList hashes;
        long hashBlockSize;
        long length;
    };

    void processPatch(const QueuedPatch &patch);

    QVector<QueuedPatch> patchQueue;

    QDir patchesDir;
    QString baseDirectory;
    BootData *boot_data = nullptr;
    GameData *game_data = nullptr;

    int remainingPatches = -1;

    LauncherCore &m_launcher;
};