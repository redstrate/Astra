#include "squareboot.h"

#include <QUrlQuery>
#include <QNetworkReply>
#include <QMessageBox>
#include <QPushButton>

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
        QString response = reply->readAll();
        if(response.isEmpty()) {
            launcher.getStored(info);
        } else {
            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Failed to launch. The game may require an update, please use another launcher.");
            window.addUpdateButtons(*info.settings, *messageBox);

            messageBox->show();
        }
    });
}