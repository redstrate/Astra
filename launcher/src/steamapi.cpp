// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "steamapi.h"
#include "encryptedarg.h"
#include "launchercore.h"
#include "utility.h"

#include <QCoroNetwork>
#include <QJsonDocument>
#include <QJsonValue>
#include <QUrlQuery>

SteamAPI::SteamAPI(QObject *parent)
    : QObject(parent)
{
}

QCoro::Task<> SteamAPI::initialize(const bool freeTrial)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("ft"), QString::number(freeTrial ? 1 : 0));

    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(50481);
    url.setPath(QStringLiteral("/init"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout();
    Utility::printRequest(QStringLiteral("GET"), request);

    Q_UNUSED(co_await m_qnam.post(request, QByteArray{}))
}

QCoro::Task<> SteamAPI::shutdown()
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(50481);
    url.setPath(QStringLiteral("/shutdown"));

    QNetworkRequest request(url);
    request.setTransferTimeout();
    Utility::printRequest(QStringLiteral("GET"), request);

    Q_UNUSED(co_await m_qnam.post(request, QByteArray{}))
}

QCoro::Task<std::pair<QString, int>> SteamAPI::getTicket()
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(50481);
    url.setPath(QStringLiteral("/ticket"));

    QNetworkRequest request(url);
    request.setTransferTimeout();
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await m_qnam.get(request);
    const auto ticketBytes = reply->readAll();

    const QJsonDocument document = QJsonDocument::fromJson(ticketBytes);

    co_return encryptSteamTicket(document[QStringLiteral("ticket")].toString(), document[QStringLiteral("time")].toInteger());
}
