// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

class LauncherCore;

class CompatibilityToolInstaller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.createCompatInstaller")

    Q_PROPERTY(bool isInstalled READ isInstalled NOTIFY isInstalledChanged)
    Q_PROPERTY(bool hasSteam READ hasSteam CONSTANT)

public:
    explicit CompatibilityToolInstaller(LauncherCore &launcher, QObject *parent = nullptr);

    Q_INVOKABLE void installCompatibilityTool();
    Q_INVOKABLE void removeCompatibilityTool();

    bool isInstalled() const;
    bool hasSteam() const;

Q_SIGNALS:
    void installFinished();
    void error(QString message);
    void removalFinished();
    void isInstalledChanged();

private:
    enum class SteamType {
        NotFound,
        Native,
        Flatpak,
    };
    std::pair<SteamType, QDir> findSteamType() const;

    LauncherCore &m_launcher;
};
