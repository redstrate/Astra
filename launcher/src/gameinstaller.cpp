// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <QFile>
#include <QNetworkReply>
#include <QStandardPaths>
#include <physis.hpp>

#include "launchercore.h"
#include "profile.h"

GameInstaller::GameInstaller(LauncherCore &launcher, Profile &profile, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
    , m_profile(profile)
{
}

void GameInstaller::installGame()
{
    const QString installDirectory = m_profile.gamePath();
    qDebug() << "Installing game to " << installDirectory << "!";
    qDebug() << "Now downloading installer file...";

    QNetworkRequest request(QUrl("https://gdl.square-enix.com/ffxiv/inst/ffxivsetup.exe"));

    auto reply = m_launcher.mgr->get(request);
    QObject::connect(reply, &QNetworkReply::finished, [this, reply, installDirectory] {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        QFile file(dataDir + "/ffxivsetup.exe");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        const std::string installDirectoryStd = installDirectory.toStdString();
        const std::string fileNameStd = file.fileName().toStdString();

        physis_install_game(fileNameStd.c_str(), installDirectoryStd.c_str());

        qDebug() << "Done installing to " << installDirectory << "!";

        Q_EMIT installFinished();
    });
}
