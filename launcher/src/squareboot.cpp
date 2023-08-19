// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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
    qDebug() << "Performing boot check...";

    patcher = new Patcher(window, info.profile->gamePath() + "/boot", info.profile->bootData, this);
    connect(patcher, &Patcher::done, [this, &info] {
        info.profile->readGameVersion();

        launcher.getStored(info);
    });

    QUrlQuery query;
    query.addQueryItem("time", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH-mm"));

    QUrl url;
    url.setScheme("http");
    url.setHost(QStringLiteral("patch-bootver.%1").arg(window.squareEnixServer()));
    url.setPath(QString("/http/win32/ffxivneo_release_boot/%1").arg(info.profile->bootVersion));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    if (info.profile->account()->license() == Account::GameLicense::macOS) {
        request.setRawHeader("User-Agent", "FFXIV-MAC PATCH CLIENT");
    } else {
        request.setRawHeader("User-Agent", "FFXIV PATCH CLIENT");
    }

    request.setRawHeader("Host", QStringLiteral("patch-bootver.%1").arg(window.squareEnixServer()).toUtf8());

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply] {
        const QString response = reply->readAll();

        patcher->processPatchList(*window.mgr, response);
    });
}

void SquareBoot::checkGateStatus(LoginInformation *info)
{
    Q_EMIT window.stageChanged(i18n("Checking gate..."));
    qDebug() << "Checking gate...";

    QUrl url;
    url.setScheme("https");
    url.setHost(QStringLiteral("frontier.%1").arg(window.squareEnixServer()));
    url.setPath("/worldStatus/gate_status.json");
    url.setQuery(QString::number(QDateTime::currentMSecsSinceEpoch()));

    QNetworkRequest request(url);

    // TODO: really?
    window.buildRequest(*info->profile, request);

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, info] {
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        const bool isGateOpen = !document.isEmpty() && document.object()["status"].toInt() != 0;

        if (isGateOpen) {
            bootCheck(*info);
        } else {
            Q_EMIT window.loginError(i18n("The login gate is closed, the game may be under maintenance.\n\n%1", reply->errorString()));
        }
    });
}
