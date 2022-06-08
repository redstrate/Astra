#include "gameinstaller.h"

#include <installextract.h>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QFile>

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

        extractBootstrapFiles((dataDir + "/ffxivsetup.exe").toStdString(), installDirectory.toStdString());

        qDebug() << "Done installing to " << installDirectory << "!";

        returnFunc();
    });
}