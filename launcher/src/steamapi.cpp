// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "steamapi.h"
#include "encryptedarg.h"
#include "launchercore.h"

#include <QCoroNetwork>

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

    Q_UNUSED(co_await m_qnam.post(QNetworkRequest(url), QByteArray{}))
}

QCoro::Task<> SteamAPI::shutdown()
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(50481);
    url.setPath(QStringLiteral("/shutdown"));

    Q_UNUSED(co_await m_qnam.post(QNetworkRequest(url), QByteArray{}))
}

QCoro::Task<std::pair<QString, int>> SteamAPI::getTicket()
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(50481);
    url.setPath(QStringLiteral("/ticket"));

    const auto reply = co_await m_qnam.get(QNetworkRequest(url));
    const auto ticketBytes = reply->readAll();

    const QJsonDocument document = QJsonDocument::fromJson(ticketBytes);

    co_return encryptSteamTicket(document[QStringLiteral("ticket")].toString(), document[QStringLiteral("time")].toInteger());
}
