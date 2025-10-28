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

    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 downloadedBytes READ downloadedBytes NOTIFY downloadedBytesChanged)

public:
    BenchmarkInstaller(LauncherCore &launcher, Profile &profile, QObject *parent = nullptr);
    BenchmarkInstaller(LauncherCore &launcher, Profile &profile, const QString &filePath, QObject *parent = nullptr);

    Q_INVOKABLE void start();

    qint64 totalBytes() const;
    qint64 downloadedBytes() const;

Q_SIGNALS:
    void installFinished();
    void error(QString message);
    void totalBytesChanged();
    void downloadedBytesChanged();

private:
    void installGame();

    LauncherCore &m_launcher;
    Profile &m_profile;
    QString m_localInstallerPath;
    qint64 m_totalBytes = 0;
    qint64 m_downloadedBytes = 0;
};
