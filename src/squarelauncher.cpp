#include "squarelauncher.h"

#include <QFile>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QRegularExpressionMatch>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QDesktopServices>

#include "launchercore.h"

#ifdef ENABLE_WATCHDOG
#include "watchdog.h"
#endif

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
    // en is always used to the top url
    query.addQueryItem("lng", "en");
    // for some reason, we always use region 3. the actual region is acquired later
    query.addQueryItem("rgn", "3");
    query.addQueryItem("isft", info.settings->license == GameLicense::FreeTrial ? "1" : "0");
    query.addQueryItem("cssmode", "1");
    query.addQueryItem("isnew", "1");
    query.addQueryItem("launchver", "3");

    if(info.settings->license == GameLicense::WindowsSteam) {
        query.addQueryItem("issteam", "1");

        // TODO: get steam ticket information from steam api
        query.addQueryItem("session_ticket", "1");
        query.addQueryItem("ticket_size", "1");
    }

    QUrl url("https://ffxiv-login.square-enix.com/oauth/ffxivarr/login/top");
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    window.buildRequest(request);

    QNetworkReply* reply = window.mgr->get(request);

    connect(reply, &QNetworkReply::finished, [=] {
        auto str = QString(reply->readAll());

        // fetches Steam username
        if(info.settings->license == GameLicense::WindowsSteam) {
            QRegularExpression re(R"lit(<input name=""sqexid"" type=""hidden"" value=""(?<sqexid>.*)""\/>)lit");
            QRegularExpressionMatch match = re.match(str);

            if(match.hasMatch()) {
                username = match.captured(1);
            } else {
                auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Could not get Steam username, have you attached your account?");
                messageBox->show();
            }
        } else {
            username = info.username;
        }

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

            if(!playable) {
                auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Your game is unplayable. Please check that you have the right license selected, and a subscription to play.");

                auto launcherButton = messageBox->addButton("Open Mog Station", QMessageBox::HelpRole);
                connect(launcherButton, &QPushButton::clicked, [=] {
                    QDesktopServices::openUrl(QUrl("https://sqex.to/Msp"));
                });

                messageBox->addButton(QMessageBox::StandardButton::Ok);

                messageBox->show();

                return;
            }

            if(!terms) {
                auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Your game is unplayable. You need to accept the terms of service from the official launcher.");
                window.addUpdateButtons(*info.settings, *messageBox);

                messageBox->show();

                return;
            }

            SID = parts[1];
            auth.region = parts[5].toInt();
            auth.maxExpansion = parts[13].toInt();

            registerSession(info);
        } else {
            QRegularExpression re(R"lit(window.external.user\("login=auth,ng,err,(?<launchParams>.*)\);)lit");
            QRegularExpressionMatch match = re.match(str);

            const auto parts = match.captured(1).split(',');

            // there's a stray quote at the end of the error string, so let's remove that
            QString errorStr = match.captured(1).chopped(1);

            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", errorStr);
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

    for(int i = 0; i < info.settings->expansionVersions.size(); i++)
        report += QString("\nex%1\t%2").arg(QString::number(i + 1), info.settings->expansionVersions[i]);

    auto reply = window.mgr->post(request, report.toUtf8());
    connect(reply, &QNetworkReply::finished, [=] {
        if(reply->rawHeaderList().contains("X-Patch-Unique-Id")) {
            auth.SID = reply->rawHeader("X-Patch-Unique-Id");

#ifdef ENABLE_WATCHDOG
            if(info.settings->enableWatchdog) {
                window.watchdog->launchGame(*info.settings, auth);
            } else {
                window.launchGame(*info.settings, auth);
            }
#else
            window.launchGame(*info.settings, auth);
#endif
        } else {
            auto messageBox = new QMessageBox(QMessageBox::Icon::Critical, "Failed to Login", "Failed the anti-tamper check. Please restore your game to the original state or update the game.");
            window.addUpdateButtons(*info.settings, *messageBox);

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
        // I happen to run into this issue often, if I start the launcher really quickly after bootup
        // it's possible to actually check this quicker than the network is actually available,
        // causing the launcher to be stuck in "maintenace mode". so if that happens, we try to rerun this logic.
        // TODO: this selection of errors is currently guesswork, i'm assuming one of these will fit the bill of "internet is unavailable" in
        // some way.
        if(reply->error() == QNetworkReply::HostNotFoundError || reply->error() == QNetworkReply::TimeoutError || reply->error() == QNetworkReply::UnknownServerError)
            gateOpen();

        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

        isGateOpen = !document.isEmpty() && document.object()["status"].toInt() != 0;

        gateStatusRecieved(isGateOpen);
    });
}
