// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sapphirelauncher.h"

#include <KLocalizedString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

SapphireLauncher::SapphireLauncher(LauncherCore &window, QObject *parent)
    : QObject(parent)
    , window(window)
{
}

void SapphireLauncher::login(const QString &lobbyUrl, const LoginInformation &info)
{
    const QJsonObject data{{QStringLiteral("username"), info.username}, {QStringLiteral("pass"), info.password}};

    const QUrl url(lobbyUrl + QStringLiteral("/sapphire-api/lobby/login"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    const auto reply = window.mgr->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [this, reply, &info] {
        if (reply->error() != QNetworkReply::NetworkError::NoError) {
            Q_EMIT window.loginError(i18n("Could not contact lobby server.\n\n%1", reply->errorString()));
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (!document.isEmpty()) {
            LoginAuth auth;
            auth.SID = document[QLatin1String("sId")].toString();
            auth.lobbyhost = document[QLatin1String("lobbyHost")].toString();
            auth.frontierHost = document[QLatin1String("frontierHost")].toString();
            auth.region = 3;

            window.launchGame(*info.profile, auth);
        } else {
            Q_EMIT window.loginError(i18n("Invalid username or password."));
        }
    });
}

void SapphireLauncher::registerAccount(const QString &lobbyUrl, const LoginInformation &info)
{
    const QJsonObject data{{QStringLiteral("username"), info.username}, {QStringLiteral("pass"), info.password}};
    const QUrl url(lobbyUrl + QStringLiteral("/sapphire-api/lobby/createAccount"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    const auto reply = window.mgr->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [&] {
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        LoginAuth auth;
        auth.SID = document[QLatin1String("sId")].toString();
        auth.lobbyhost = document[QLatin1String("lobbyHost")].toString();
        auth.frontierHost = document[QLatin1String("frontierHost")].toString();
        auth.region = 3;

        window.launchGame(*info.profile, auth);
    });
}