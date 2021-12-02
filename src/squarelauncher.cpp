#include "squarelauncher.h"

#include <QFile>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QRegularExpressionMatch>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

#include "launchercore.h"

SquareLauncher::SquareLauncher(LauncherCore& window) : window(window) {

}

QString getFileHash(QString file) {
    auto f = QFile(file);
    if (!f.open(QIODevice::ReadOnly))
        return "";

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&f);

    return QString("%1/%2").arg(QString::number(f.size()), hash.result().toHex());
}

void SquareLauncher::getStored(const LoginInformation& info) {
    QUrlQuery query;
    query.addQueryItem("lng", "en");
    query.addQueryItem("rgn", "3");
    query.addQueryItem("isft", "0");
    query.addQueryItem("cssmode", "1");
    query.addQueryItem("isnew", "1");
    query.addQueryItem("issteam", "0");

    QUrl url("https://ffxiv-login.square-enix.com/oauth/ffxivarr/login/top");
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    window.buildRequest(request);

    QNetworkReply* reply = window.mgr->get(request);

    connect(reply, &QNetworkReply::finished, [=] {
        auto str = QString(reply->readAll());

        QRegularExpression re(R"lit(\t<\s*input .* name="_STORED_" value="(?<stored>.*)">)lit");
        QRegularExpressionMatch match = re.match(str);
        if (match.hasMatch()) {
            stored = match.captured(1);
            login(info, url);
        } else {
            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Failed to contact SE servers. They may be in maintenance.");
            messageBox->show();
        }
    });
}

void SquareLauncher::login(const LoginInformation& info, const QUrl referer) {
    QUrlQuery postData;
    postData.addQueryItem("_STORED_", stored);
    postData.addQueryItem("sqexid", info.username);
    postData.addQueryItem("password", info.password);
    postData.addQueryItem("otppw", info.oneTimePassword);

    QNetworkRequest request(QUrl("https://ffxiv-login.square-enix.com/oauth/ffxivarr/login/login.send"));
    window.buildRequest(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setRawHeader("Referer", referer.toEncoded());
    request.setRawHeader("Cache-Control", "no-cache");

    auto reply = window.mgr->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, [=] {
        auto str = QString(reply->readAll());

        QRegularExpression re(R"lit(window.external.user\("login=auth,ok,(?<launchParams>.*)\);)lit");
        QRegularExpressionMatch match = re.match(str);
        if(match.hasMatch()) {
            const auto parts = match.captured(1).split(',');

            const bool terms = parts[3] == "1";
            const bool playable = parts[9] == "1";

            if(!terms || !playable) {
                auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Your game is unplayable. You may need to accept the terms from the official launcher.");
                messageBox->show();
            } else {
                SID = parts[1];
                auth.region = parts[5].toInt();
                auth.maxExpansion = parts[13].toInt();

                readExpansionVersions(info, auth.maxExpansion);

                registerSession(info);
            }
        } else {
            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Invalid username/password.");
            messageBox->show();
        }
    });
}

void SquareLauncher::registerSession(const LoginInformation& info) {
    QUrl url;
    url.setScheme("https");
    url.setHost("patch-gamever.ffxiv.com");
    url.setPath(QString("/http/win32/ffxivneo_release_game/%1/%2").arg(info.settings->gameVersion, SID));

    auto request = QNetworkRequest(url);
    window.setSSL(request);
    request.setRawHeader("X-Hash-Check", "enabled");
    request.setRawHeader("User-Agent", "FFXIV PATCH CLIENT");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QString report = info.settings->bootVersion + "=" + getBootHash(info);

    for(int i = 0; i < expansionVersions.size(); i++)
        report += QString("\nex%1\t%2").arg(QString::number(i + 1), expansionVersions[i]);

    auto reply = window.mgr->post(request, report.toUtf8());
    connect(reply, &QNetworkReply::finished, [=] {
        if(reply->rawHeaderList().contains("X-Patch-Unique-Id")) {
            auth.SID = reply->rawHeader("X-Patch-Unique-Id");

            window.launchGame(*info.settings, auth);
        } else {
            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Failed the anti-tamper check. Please restore your game to the original state or update the game.");
            messageBox->show();
        }
    });
}

QString SquareLauncher::getBootHash(const LoginInformation& info) {
    const QList<QString> fileList =
            {
                    "ffxivboot.exe",
                    "ffxivboot64.exe",
                    "ffxivlauncher.exe",
                    "ffxivlauncher64.exe",
                    "ffxivupdater.exe",
                    "ffxivupdater64.exe"
            };

    QString result;
    for (int i = 0; i < fileList.count(); i++) {
        result += fileList[i] + "/" + getFileHash(info.settings->gamePath + "/boot/" + fileList[i]);

        if (i != fileList.length() - 1)
            result += ",";
    }

    return result;
}

void SquareLauncher::readExpansionVersions(const LoginInformation& info, int max) {
    expansionVersions.clear();

    for(int i = 0; i < max; i++)
        expansionVersions.push_back(window.readVersion(QString("%1/game/sqpack/ex%2/ex%2.ver").arg(info.settings->gamePath, QString::number(i + 1))));
}

void SquareLauncher::gateOpen() {
    QUrlQuery query;
    query.addQueryItem("", QString::number(QDateTime::currentMSecsSinceEpoch()));

    QUrl url;
    url.setUrl("https://frontier.ffxiv.com/worldStatus/gate_status.json");
    url.setQuery(query);

    QNetworkRequest request;
    request.setUrl(url);
    window.buildRequest(request);

    auto reply = window.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply] {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        if(document.isEmpty() || document.object()["status"].toInt() == 0) {
            gateStatusRecieved(false);
            isGateOpen = false;
        } else {
            gateStatusRecieved(true);
            isGateOpen = true;
        }
    });
}