#include "gameinstaller.h"

#include <QNetworkReply>
#include <QStandardPaths>
#include <QFile>
#include <physis.hpp>

#include "launchercore.h"

void installGame(LauncherCore& launcher, ProfileSettings& profile, std::function<void()> returnFunc) {
    QString installDirectory = profile.gamePath;
    qDebug() << "Installing game to " << installDirectory << "!";

    qDebug() << "Now downloading installer file...";

    QNetworkRequest request(QUrl("https://gdl.square-enix.com/ffxiv/inst/ffxivsetup.exe"));

    auto reply = launcher.mgr->get(request);
    launcher.connect(reply, &QNetworkReply::finished, [reply, installDirectory, returnFunc] {
        QString dataDir =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        QFile file(dataDir + "/ffxivsetup.exe");
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();

        physis_install_game(installDirectory.toStdString().c_str(), (dataDir + "/ffxivsetup.exe").toStdString().c_str());

        qDebug() << "Done installing to " << installDirectory << "!";

        returnFunc();
    });
}