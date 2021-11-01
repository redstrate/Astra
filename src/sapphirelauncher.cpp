#include "sapphirelauncher.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkReply>

SapphireLauncher::SapphireLauncher(LauncherWindow& window) : window(window) {

}

void SapphireLauncher::login(QString lobbyUrl, const LoginInformation& info) {
    QJsonObject data {
            {"username", info.username},
            {"pass", info.password}
    };

    QUrl url;
    url.setScheme("http");
    url.setHost(lobbyUrl);
    url.setPath("/sapphire-api/lobby/login");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    auto reply = window.mgr->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [=] {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if(!document.isEmpty()) {
            LoginAuth auth;
            auth.SID = document["sId"].toString();
            auth.lobbyhost = document["lobbyHost"].toString();
            auth.frontierHost = document["frontierHost"].toString();
            auth.region = 3;

            window.launch(auth);
        } else {
            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Invalid username/password.");
            messageBox->show();
        }
    });
}

void SapphireLauncher::registerAccount(QString lobbyUrl, const LoginInformation& info) {
    QJsonObject data {
            {"username", info.username},
            {"pass", info.password}
    };
    QUrl url;
    url.setScheme("http");
    url.setHost(lobbyUrl);
    url.setPath("/sapphire-api/lobby/createAccount");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    auto reply = window.mgr->post(request, QJsonDocument(data).toJson(QJsonDocument::JsonFormat::Compact));
    connect(reply, &QNetworkReply::finished, [=] {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        LoginAuth auth;
        auth.SID = document["sId"].toString();
        auth.lobbyhost = document["lobbyHost"].toString();
        auth.frontierHost = document["frontierHost"].toString();
        auth.region = 3;

        window.launch(auth);
    });
}