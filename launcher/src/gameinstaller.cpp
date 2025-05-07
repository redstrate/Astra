// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <QFile>

#include "astra_log.h"
#include "launchercore.h"
#include "profile.h"
#include "profileconfig.h"
#include "utility.h"

GameInstaller::GameInstaller(LauncherCore &launcher, Profile &profile, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
    , m_profile(profile)
{
}

GameInstaller::GameInstaller(LauncherCore &launcher, Profile &profile, const QString &filePath, QObject *parent)
    : GameInstaller(launcher, profile, parent)
{
    m_localInstallerPath = filePath;
}

void GameInstaller::start()
{
    installGame();
}

void GameInstaller::installGame()
{
    const QDir installDirectory = m_profile.config()->gamePath();

    const QDir bootDir = installDirectory.absoluteFilePath(QStringLiteral("boot"));
    if (!bootDir.exists()) {
        QDir().mkpath(bootDir.path());
    }

    const QDir gameDir = installDirectory.absoluteFilePath(QStringLiteral("game"));
    if (!gameDir.exists()) {
        QDir().mkpath(gameDir.path());
    }

    QFile bootVerFile(bootDir.absoluteFilePath(QStringLiteral("ffxivboot.ver")));
    bootVerFile.open(QIODevice::WriteOnly);
    bootVerFile.write(QByteArrayLiteral("2012.01.01.0000.0000"));

    QFile gameVerFile(gameDir.absoluteFilePath(QStringLiteral("ffxivgame.ver")));
    gameVerFile.open(QIODevice::WriteOnly);
    gameVerFile.write(QByteArrayLiteral("2012.01.01.0000.0000"));

    m_profile.readGameVersion();

    Q_EMIT installFinished();
    qInfo(ASTRA_LOG) << "Installed game in" << installDirectory;
}

#include "moc_gameinstaller.cpp"
