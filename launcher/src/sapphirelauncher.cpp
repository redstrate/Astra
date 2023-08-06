// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sapphirelauncher.h"

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
    QJsonObject data{{"username", info.username}, {"pass", info.password}};

    QUrl url(lobbyUrl + "/sapphire-api/lobby/login");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    auto reply = window.mgr->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [&] {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (!document.isEmpty()) {
            LoginAuth auth;
            auth.SID = document["sId"].toString();
            auth.lobbyhost = document["lobbyHost"].toString();
            auth.frontierHost = document["frontierHost"].toString();
            auth.region = 3;

            window.launchGame(*info.profile, auth);
        } else {
            /*auto messageBox =
                new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Invalid username/password.");
            messageBox->show();*/
        }
    });
}

void SapphireLauncher::registerAccount(const QString &lobbyUrl, const LoginInformation &info)
{
    QJsonObject data{{"username", info.username}, {"pass", info.password}};
    QUrl url(lobbyUrl + "/sapphire-api/lobby/createAccount");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    auto reply = window.mgr->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [&] {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        LoginAuth auth;
        auth.SID = document["sId"].toString();
        auth.lobbyhost = document["lobbyHost"].toString();
        auth.frontierHost = document["frontierHost"].toString();
        auth.region = 3;

        window.launchGame(*info.profile, auth);
    });
}