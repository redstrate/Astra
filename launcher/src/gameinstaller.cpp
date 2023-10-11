// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <KLocalizedString>
#include <QFile>
#include <QNetworkReply>
#include <QStandardPaths>
#include <physis.hpp>

#include "astra_log.h"
#include "launchercore.h"
#include "profile.h"
#include "utility.h"

const QString installerUrl = QStringLiteral("https://download.finalfantasyxiv.com/inst/ffxivsetup.exe");
const QByteArray installerSha256 = QByteArray::fromHex("cf70bfaaf4f429794358ef84acbcbdc4193bee109fa1b6aea81bd4de038e500e");

GameInstaller::GameInstaller(LauncherCore &launcher, Profile &profile, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
    , m_profile(profile)
{
}

void GameInstaller::installGame()
{
    const QDir installDirectory = m_profile.gamePath();

    QNetworkRequest request((QUrl(installerUrl)));

    auto reply = m_launcher.mgr()->get(request);
    Utility::printRequest(QStringLiteral("GET"), request);

    QObject::connect(reply, &QNetworkReply::finished, [this, reply, installDirectory] {
        if (reply->error() != QNetworkReply::NetworkError::NoError) {
            Q_EMIT error(i18n("An error has occurred when downloading the installer.\n\n%1", reply->errorString()));
            return;
        }

        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        const QByteArray data = reply->readAll();
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(data);

        // TODO: turn into a proper error
        Q_ASSERT(hash.result() == installerSha256);

        QFile file(dataDir.absoluteFilePath("ffxivsetup.exe"));
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();

        const std::string installDirectoryStd = installDirectory.absolutePath().toStdString();
        const std::string fileNameStd = file.fileName().toStdString();

        physis_install_game(fileNameStd.c_str(), installDirectoryStd.c_str());

        m_profile.readGameVersion();

        Q_EMIT installFinished();
        qInfo(ASTRA_LOG) << "Installed game in" << installDirectory;
    });
}
