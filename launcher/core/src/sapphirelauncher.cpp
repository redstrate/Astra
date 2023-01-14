#include "sapphirelauncher.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>

SapphireLauncher::SapphireLauncher(LauncherCore& window) : window(window), QObject(&window) {}

void SapphireLauncher::login(const QString& lobbyUrl, const LoginInformation& info) {
    QJsonObject data{{"username", info.username}, {"pass", info.password}};

    QUrl url;
    url.setScheme("http");
    url.setHost(lobbyUrl);
    url.setPath("/sapphire-api/lobby/login");

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

            window.launchGame(*info.settings, auth);
        } else {
            auto messageBox =
                new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Invalid username/password.");
            messageBox->show();
        }
    });
}

void SapphireLauncher::registerAccount(const QString& lobbyUrl, const LoginInformation& info) {
    QJsonObject data{{"username", info.username}, {"pass", info.password}};
    QUrl url(lobbyUrl + "/sapphire-api/lobby/login");

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

        window.launchGame(*info.settings, auth);
    });
}
