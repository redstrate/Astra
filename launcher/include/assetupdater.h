// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonArray>
#include <QObject>
#include <QTemporaryDir>

#include "launchercore.h"

class LauncherCore;
class QNetworkReply;

class AssetUpdater : public QObject
{
    Q_OBJECT
public:
    explicit AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent = nullptr);

    void update();
    void beginInstall();

    void checkIfCheckingIsDone();
    void checkIfDalamudAssetsDone();
    void checkIfFinished();

signals:
    void finishedUpdating();

private:
    LauncherCore &launcher;

    Profile::DalamudChannel chosenChannel;

    QString remoteDalamudVersion;
    QString remoteRuntimeVersion;

    QTemporaryDir tempDir;

    bool doneDownloadingDalamud = false;
    bool doneDownloadingRuntimeCore = false;
    bool doneDownloadingRuntimeDesktop = false;
    bool needsRuntimeInstall = false;
    bool needsDalamudInstall = false;

    int remoteDalamudAssetVersion = -1;
    QList<QString> dalamudAssetNeededFilenames;
    QJsonArray remoteDalamudAssetArray;

    QString dataDir;
    Profile &m_profile;
};
