#include "gameinstaller.h"

#include <QFile>
#include <QNetworkReply>
#include <QStandardPaths>
#include <physis.hpp>

#include "launchercore.h"

void installGame(LauncherCore& launcher, ProfileSettings& profile, const std::function<void()>& returnFunc) {
    QString installDirectory = profile.gamePath;
    qDebug() << "Installing game to " << installDirectory << "!";

    qDebug() << "Now downloading installer file...";

    QNetworkRequest request(QUrl("https://gdl.square-enix.com/ffxiv/inst/ffxivsetup.exe"));

    auto reply = launcher.mgr->get(request);
    QObject::connect(reply, &QNetworkReply::finished, [reply, installDirectory, returnFunc] {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        QFile file(dataDir + "/ffxivsetup.exe");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        const std::string installDirectoryStd = installDirectory.toStdString();
        const std::string fileNameStd = file.fileName().toStdString();

        physis_install_game(fileNameStd.c_str(), installDirectoryStd.c_str());

        qDebug() << "Done installing to " << installDirectory << "!";

        returnFunc();
    });
}