// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sapphirelogin.h"
#include "utility.h"

#include <KLocalizedString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <qcoronetwork.h>

using namespace Qt::StringLiterals;

SapphireLogin::SapphireLogin(LauncherCore &window, QObject *parent)
    : QObject(parent)
    , m_launcher(window)
{
}

QCoro::Task<std::optional<LoginAuth>> SapphireLogin::login(const QString &lobbyUrl, const LoginInformation &info)
{
    const QJsonObject data{{QStringLiteral("username"), info.username}, {QStringLiteral("pass"), info.password}};

    const QUrl url(lobbyUrl + QStringLiteral("/sapphire-api/lobby/login"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));
    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = m_launcher.mgr()->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT m_launcher.loginError(i18n("Could not contact lobby server.\n\n%1", reply->errorString()));
        co_return std::nullopt;
    }

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isEmpty()) {
        LoginAuth auth;
        auth.SID = document["sId"_L1].toString();
        auth.lobbyhost = document["lobbyHost"_L1].toString();
        auth.frontierHost = document["frontierHost"_L1].toString();
        auth.region = 3;

        co_return auth;
    } else {
        Q_EMIT m_launcher.loginError(i18n("Invalid username or password."));
        co_return std::nullopt;
    }
}

void SapphireLogin::registerAccount(const QString &lobbyUrl, const LoginInformation &info)
{
    const QJsonObject data{{QStringLiteral("username"), info.username}, {QStringLiteral("pass"), info.password}};
    const QUrl url(lobbyUrl + QStringLiteral("/sapphire-api/lobby/createAccount"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = m_launcher.mgr()->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [&] {
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        LoginAuth auth;
        auth.SID = document["sId"_L1].toString();
        auth.lobbyhost = document["lobbyHost"_L1].toString();
        auth.frontierHost = document["frontierHost"_L1].toString();
        auth.region = 3;

        // m_launcher.launchGame(*info.profile, auth);
    });
}