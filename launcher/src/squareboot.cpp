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
#include <qcoronetworkreply.h>

#include "account.h"
#include "squarelauncher.h"
#include "utility.h"

SquareBoot::SquareBoot(LauncherCore &window, SquareLauncher &launcher, QObject *parent)
    : QObject(parent)
    , window(window)
    , launcher(launcher)
{
}

QCoro::Task<> SquareBoot::bootCheck(const LoginInformation &info)
{
    Q_EMIT window.stageChanged(i18n("Checking for launcher updates..."));
    qDebug() << "Performing boot check...";

    const QUrlQuery query{{QStringLiteral("time"), QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd-HH-mm"))}};

    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("patch-bootver.%1").arg(window.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/http/win32/ffxivneo_release_boot/%1").arg(info.profile->bootVersion()));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    if (info.profile->account()->license() == Account::GameLicense::macOS) {
        request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("FFXIV-MAC PATCH CLIENT"));
    } else {
        request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("FFXIV PATCH CLIENT"));
    }

    request.setRawHeader(QByteArrayLiteral("Host"), QStringLiteral("patch-bootver.%1").arg(window.settings()->squareEnixServer()).toUtf8());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = window.mgr()->get(request);
    co_await reply;

    const QString patchList = reply->readAll();
    if (!patchList.isEmpty()) {
        patcher = new Patcher(window, info.profile->gamePath() + QStringLiteral("/boot"), *info.profile->bootData(), this);
        const bool hasPatched = co_await patcher->patch(PatchList(patchList));
        if (hasPatched) {
            // update game version information
            info.profile->readGameVersion();
        }
        patcher->deleteLater();
    }

    launcher.login(info);
}

QCoro::Task<> SquareBoot::checkGateStatus(const LoginInformation &info)
{
    Q_EMIT window.stageChanged(i18n("Checking gate..."));
    qDebug() << "Checking gate...";

    QUrl url;
    url.setScheme(window.settings()->preferredProtocol());
    url.setHost(QStringLiteral("frontier.%1").arg(window.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/worldStatus/gate_status.json"));
    url.setQuery(QString::number(QDateTime::currentMSecsSinceEpoch()));

    QNetworkRequest request(url);

    // TODO: really?
    window.buildRequest(*info.profile, request);

    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = window.mgr()->get(request);
    window.setupIgnoreSSL(reply);
    co_await reply;

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    const bool isGateOpen = !document.isEmpty() && document.object()[QLatin1String("status")].toInt() != 0;

    if (isGateOpen) {
        bootCheck(info);
    } else {
        Q_EMIT window.loginError(i18n("The login gate is closed, the game may be under maintenance.\n\n%1", reply->errorString()));
    }
}
