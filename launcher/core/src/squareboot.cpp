#include "squareboot.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QStandardPaths>
#include <QUrlQuery>
#include <physis.hpp>

#include "squarelauncher.h"

SquareBoot::SquareBoot(LauncherCore& window, SquareLauncher& launcher)
    : window(window), launcher(launcher), QObject(&window) {}

void SquareBoot::bootCheck(const LoginInformation& info) {
    patcher = new Patcher(info.settings->gamePath + "/boot", info.settings->bootData);
    connect(patcher, &Patcher::done, [=, &info] {
        window.readGameVersion();

        launcher.getStored(info);
    });

    QUrlQuery query;
    query.addQueryItem("time", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH-mm"));

    QUrl url;
    url.setScheme("http");
    url.setHost("patch-bootver.ffxiv.com");
    url.setPath(QString("/http/win32/ffxivneo_release_boot/%1").arg(info.settings->bootVersion));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    if (info.settings->license == GameLicense::macOS) {
        request.setRawHeader("User-Agent", "FFXIV-MAC PATCH CLIENT");
    } else {
        request.setRawHeader("User-Agent", "FFXIV PATCH CLIENT");
    }

    request.setRawHeader("Host", "patch-bootver.ffxiv.com");

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [=, &info] {
        const QString response = reply->readAll();

        patcher->processPatchList(*window.mgr, response);
    });
}

void SquareBoot::checkGateStatus(LoginInformation* info) {
    QUrlQuery query;
    query.addQueryItem("", QString::number(QDateTime::currentMSecsSinceEpoch()));

    QUrl url;
    url.setUrl("https://frontier.ffxiv.com/worldStatus/gate_status.json");
    url.setQuery(query);

    QNetworkRequest request;
    request.setUrl(url);

    // TODO: really?
    window.buildRequest(*info->settings, request);

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [=] {
        // I happen to run into this issue often, if I start the launcher really quickly after bootup
        // it's possible to actually check this quicker than the network is actually available,
        // causing the launcher to be stuck in "maintenace mode". so if that happens, we try to rerun this logic.
        // TODO: this selection of errors is currently guesswork, i'm assuming one of these will fit the bill of
        // "internet is unavailable" in some way.
        if (reply->error() == QNetworkReply::HostNotFoundError || reply->error() == QNetworkReply::TimeoutError ||
            reply->error() == QNetworkReply::UnknownServerError)
            checkGateStatus(info);

        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        const bool isGateOpen = !document.isEmpty() && document.object()["status"].toInt() != 0;

        if (isGateOpen) {
            bootCheck(*info);
        } else {
            auto messageBox = new QMessageBox(
                QMessageBox::Icon::Critical,
                "Failed to Login",
                "The login gate is closed, the game may be under maintenance.");

            messageBox->show();
        }
    });
}
