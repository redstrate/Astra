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

GameInstaller::GameInstaller(LauncherCore &launcher, Profile &profile, const QString &filePath, QObject *parent)
    : GameInstaller(launcher, profile, parent)
{
    m_localInstallerPath = filePath;
}

void GameInstaller::start()
{
    if (m_localInstallerPath.isEmpty()) {
        const QNetworkRequest request = QNetworkRequest(QUrl(installerUrl));
        Utility::printRequest(QStringLiteral("GET"), request);

        auto reply = m_launcher.mgr()->get(request);

        QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
            if (reply->error() != QNetworkReply::NetworkError::NoError) {
                Q_EMIT error(reply->errorString());
                return;
            }

            const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

            const QByteArray data = reply->readAll();
            QCryptographicHash hash(QCryptographicHash::Sha256);
            hash.addData(data);

            if (hash.result() != installerSha256) {
                Q_EMIT error(i18n("The installer failed the integrity check!"));
                return;
            }

            QFile file(dataDir.absoluteFilePath(QStringLiteral("ffxivsetup.exe")));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();

            m_localInstallerPath = file.fileName();
            installGame();
        });
    } else {
        installGame();
    }
}

void GameInstaller::installGame()
{
    const QDir installDirectory = m_profile.gamePath();

    const std::string installDirectoryStd = installDirectory.absolutePath().toStdString();
    const std::string fileNameStd = m_localInstallerPath.toStdString();

    physis_install_game(fileNameStd.c_str(), installDirectoryStd.c_str());

    m_profile.readGameVersion();

    Q_EMIT installFinished();
    qInfo(ASTRA_LOG) << "Installed game in" << installDirectory;
}

#include "moc_gameinstaller.cpp"