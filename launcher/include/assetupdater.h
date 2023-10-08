// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonArray>
#include <QObject>
#include <QTemporaryDir>
#include <qcorotask.h>

#include "launchercore.h"

class LauncherCore;
class QNetworkReply;

class AssetUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent = nullptr);

    QCoro::Task<bool> update();

private:
    QCoro::Task<bool> checkRemoteDalamudAssetVersion();
    QCoro::Task<bool> checkRemoteDalamudVersion();

    QCoro::Task<bool> installDalamudAssets();
    QCoro::Task<bool> installDalamud();
    QCoro::Task<bool> installRuntime();

    [[nodiscard]] QUrl dalamudVersionManifestUrl() const;
    [[nodiscard]] QUrl dalamudAssetManifestUrl() const;
    [[nodiscard]] QUrl dotnetRuntimePackageUrl(const QString &version) const;
    [[nodiscard]] QUrl dotnetDesktopPackageUrl(const QString &version) const;

    LauncherCore &launcher;

    Profile::DalamudChannel chosenChannel;

    QString remoteDalamudVersion;
    QString remoteRuntimeVersion;

    QTemporaryDir tempDir;
    QDir dataDir;
    QDir appDataDir;
    QDir dalamudDir;
    QDir dalamudAssetDir;
    QDir dalamudRuntimeDir;

    int remoteDalamudAssetVersion = -1;
    QJsonArray remoteDalamudAssetArray;
    QString remoteDalamudDownloadUrl;

    Profile &m_profile;
};
