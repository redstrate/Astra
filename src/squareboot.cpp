#include "squareboot.h"

#include <QUrlQuery>
#include <QNetworkReply>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QFile>
#include <patch.h>

#include "squarelauncher.h"

SquareBoot::SquareBoot(LauncherCore& window, SquareLauncher& launcher) : window(window), launcher(launcher) {

}

void SquareBoot::bootCheck(LoginInformation& info) {
    QUrlQuery query;
    query.addQueryItem("time", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH-mm"));

    QUrl url;
    url.setScheme("http");
    url.setHost("patch-bootver.ffxiv.com");
    url.setPath(QString("/http/win32/ffxivneo_release_boot/%1").arg(info.settings->bootVersion));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    if(info.settings->license == GameLicense::macOS) {
        request.setRawHeader("User-Agent", "FFXIV-MAC PATCH CLIENT");
    } else {
        request.setRawHeader("User-Agent", "FFXIV PATCH CLIENT");
    }

    request.setRawHeader("Host", "patch-bootver.ffxiv.com");

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [=] {
        const QString response = reply->readAll();

        if(response.isEmpty()) {
            launcher.getStored(info);
        } else {
            // TODO: move this out into a dedicated function, we need to use this for regular game patches later on
            // TODO: create a nice progress window like ffxivboot has
            // TODO: improve flow when updating boot, maybe do at launch ala official launcher?
            const QStringList parts = response.split(QRegExp("\n|\r\n|\r"));

            // patch list starts at line 5
            for(int i = 5; i < parts.size() - 2; i++) {
                const QStringList patchParts = parts[i].split("\t");

                const int length = patchParts[0].toInt();
                const int version = patchParts[4].toInt();
                const int hashType = patchParts[5].toInt();

                QString name = patchParts[4];
                QString url = patchParts[5];

                QNetworkRequest patchRequest(url);
                auto patchReply = window.mgr->get(patchRequest);
                connect(patchReply, &QNetworkReply::finished, [=] {
                    const QString dataDir =
                        QStandardPaths::writableLocation(QStandardPaths::TempLocation);

                    QFile file(dataDir + "/" + name + ".patch");
                    file.open(QIODevice::WriteOnly);
                    file.write(patchReply->readAll());
                    file.close();

                    // TODO: we really a dedicated .ver writing/reading class
                    QFile verFile(info.settings->gamePath + "/boot/ffxivboot.ver");
                    verFile.open(QIODevice::WriteOnly | QIODevice::Text);
                    verFile.write(name.toUtf8());
                    verFile.close();

                    processPatch((dataDir + "/" + name + ".patch").toStdString(), (info.settings->gamePath + "/boot").toStdString());

                    auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Successfully updated", "ffxivboot is now updated to " + name);

                    messageBox->show();
                });
            }
        }
    });
}