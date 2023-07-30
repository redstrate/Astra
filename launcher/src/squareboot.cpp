#include "squareboot.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <physis.hpp>

#include "account.h"
#include "squarelauncher.h"

SquareBoot::SquareBoot(LauncherCore &window, SquareLauncher &launcher, QObject *parent)
    : QObject(parent)
    , window(window)
    , launcher(launcher)
{
}

void SquareBoot::bootCheck(const LoginInformation &info)
{
    Q_EMIT window.stageChanged(i18n("Checking for launcher updates..."));

    patcher = new Patcher(info.profile->gamePath() + "/boot", info.profile->bootData, this);
    connect(patcher, &Patcher::done, [this, &info] {
        info.profile->readGameVersion();

        launcher.getStored(info);
    });

    QUrlQuery query;
    query.addQueryItem("time", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH-mm"));

    QUrl url;
    url.setScheme("http");
    url.setHost("patch-bootver.ffxiv.com");
    url.setPath(QString("/http/win32/ffxivneo_release_boot/%1").arg(info.profile->bootVersion));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    if (info.profile->account()->license() == Account::GameLicense::macOS) {
        request.setRawHeader("User-Agent", "FFXIV-MAC PATCH CLIENT");
    } else {
        request.setRawHeader("User-Agent", "FFXIV PATCH CLIENT");
    }

    request.setRawHeader("Host", "patch-bootver.ffxiv.com");

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply] {
        const QString response = reply->readAll();

        patcher->processPatchList(*window.mgr, response);
    });
}

void SquareBoot::checkGateStatus(LoginInformation *info)
{
    Q_EMIT window.stageChanged(i18n("Checking gate..."));

    QUrl url("https://frontier.ffxiv.com/worldStatus/gate_status.json");
    url.setQuery(QString::number(QDateTime::currentMSecsSinceEpoch()));

    QNetworkRequest request(url);

    // TODO: really?
    window.buildRequest(*info->profile, request);

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, info] {
        // I happen to run into this issue often, if I start the launcher really quickly after bootup
        // it's possible to actually check this quicker than the network is actually available,
        // causing the launcher to be stuck in "maintenace mode". so if that happens, we try to rerun this logic.
        // TODO: this selection of errors is currently guesswork, i'm assuming one of these will fit the bill of
        // "internet is unavailable" in some way.
        if (reply->error() == QNetworkReply::HostNotFoundError || reply->error() == QNetworkReply::TimeoutError
            || reply->error() == QNetworkReply::UnknownServerError)
            checkGateStatus(info);

        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        const bool isGateOpen = !document.isEmpty() && document.object()["status"].toInt() != 0;

        if (isGateOpen) {
            bootCheck(*info);
        } else {
            Q_EMIT window.loginError(i18n("The login gate is closed, the game may be under maintenance."));
        }
    });
}
