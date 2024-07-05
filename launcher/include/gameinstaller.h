// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

class LauncherCore;
class Profile;

class GameInstaller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.createInstaller")

public:
    GameInstaller(LauncherCore &launcher, Profile &profile, QObject *parent = nullptr);
    GameInstaller(LauncherCore &launcher, Profile &profile, const QString &filePath, QObject *parent = nullptr);

    Q_INVOKABLE void start();

Q_SIGNALS:
    void installFinished();
    void error(QString message);

private:
    void installGame();

    LauncherCore &m_launcher;
    Profile &m_profile;
    QString m_localInstallerPath;
};