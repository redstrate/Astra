// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "launchercore.h"

class LauncherCore;
class QNetworkReply;

class AssetUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent = nullptr);

    /// Checks for any asset updates. (This currently only means Dalamud.)
    /// \return False if the asset update failed, which should be considered fatal and Dalamud should not be used.
    QCoro::Task<bool> update();

private:
    QCoro::Task<bool> checkRemoteCompatibilityToolVersion();
    QCoro::Task<bool> checkRemoteDxvkVersion();

    QCoro::Task<bool> checkRemoteDalamudAssetVersion();
    QCoro::Task<bool> checkRemoteDalamudVersion();

    QCoro::Task<bool> installCompatibilityTool() const;
    QCoro::Task<bool> installDxvkTool() const;
    QCoro::Task<bool> installDalamudAssets();
    QCoro::Task<bool> installDalamud();
    QCoro::Task<bool> installRuntime();

    [[nodiscard]] QUrl dalamudVersionManifestUrl() const;
    [[nodiscard]] QUrl dalamudAssetManifestUrl() const;
    [[nodiscard]] QUrl dotnetRuntimePackageUrl(const QString &version) const;
    [[nodiscard]] QUrl dotnetDesktopPackageUrl(const QString &version) const;
    [[nodiscard]] QUrl dxvkReleasesUrl() const;
    [[nodiscard]] QUrl wineReleasesUrl() const;

    static bool extractZip(const QString &filePath, const QString &directory);

    LauncherCore &launcher;

    QString m_remoteDalamudVersion;
    QString m_remoteRuntimeVersion;

    QTemporaryDir m_tempDir;
    QDir m_wineDir;
    QDir m_dxvkDir;
    QDir m_dataDir;
    QDir m_appDataDir;
    QDir m_dalamudDir;
    QDir m_dalamudAssetDir;
    QDir m_dalamudRuntimeDir;

    int m_remoteDalamudAssetVersion = -1;
    QJsonArray m_remoteDalamudAssetArray;
    QString m_remoteDalamudDownloadUrl;
    QString m_remoteDalamudAssetPackageUrl;
    QString m_remoteCompatibilityToolVersion;
    QString m_remoteCompatibilityToolUrl;
    QString m_remoteDxvkToolVersion;
    QString m_remoteDxvkToolUrl;

    Profile &m_profile;
};
