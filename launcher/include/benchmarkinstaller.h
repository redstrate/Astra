// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QtQml>

class LauncherCore;
class Profile;

class BenchmarkInstaller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.createBenchmarkInstaller")

public:
    BenchmarkInstaller(LauncherCore &launcher, Profile &profile, QObject *parent = nullptr);
    BenchmarkInstaller(LauncherCore &launcher, Profile &profile, const QString &filePath, QObject *parent = nullptr);

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