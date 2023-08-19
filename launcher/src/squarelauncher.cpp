// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "squarelauncher.h"

#include <KLocalizedString>
#include <QDesktopServices>
#include <QFile>
#include <QNetworkReply>
#include <QRegularExpressionMatch>
#include <QUrlQuery>

#include "account.h"
#include "launchercore.h"

SquareLauncher::SquareLauncher(LauncherCore &window, QObject *parent)
    : QObject(parent)
    , window(window)
{
}

QString getFileHash(const QString &file)
{
    auto f = QFile(file);
    if (!f.open(QIODevice::ReadOnly))
        return "";

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&f);

    return QString("%1/%2").arg(QString::number(f.size()), hash.result().toHex());
}

void SquareLauncher::getStored(const LoginInformation &info)
{
    Q_EMIT window.stageChanged(i18n("Logging in..."));

    QUrlQuery query;
    // en is always used to the top url
    query.addQueryItem("lng", "en");
    // for some reason, we always use region 3. the actual region is acquired later
    query.addQueryItem("rgn", "3");
    query.addQueryItem("isft", info.profile->account()->isFreeTrial() ? "1" : "0");
    query.addQueryItem("cssmode", "1");
    query.addQueryItem("isnew", "1");
    query.addQueryItem("launchver", "3");

    if (info.profile->account()->license() == Account::GameLicense::WindowsSteam) {
        query.addQueryItem("issteam", "1");

        // TODO: get steam ticket information from steam api
        query.addQueryItem("session_ticket", "1");
        query.addQueryItem("ticket_size", "1");
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(QStringLiteral("ffxiv-login.%1").arg(window.squareEnixLoginServer()));
    url.setPath("/oauth/ffxivarr/login/top");
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    window.buildRequest(*info.profile, request);

    QNetworkReply *reply = window.mgr->get(request);

    connect(reply, &QNetworkReply::finished, [this, &info, reply, url] {
        auto str = QString(reply->readAll());

        // fetches Steam username
        if (info.profile->account()->license() == Account::GameLicense::WindowsSteam) {
            QRegularExpression re(R"lit(<input name=""sqexid"" type=""hidden"" value=""(?<sqexid>.*)""\/>)lit");
            QRegularExpressionMatch match = re.match(str);

            if (match.hasMatch()) {
                username = match.captured(1);
            } else {
                Q_EMIT window.loginError(i18n("Could not get Steam username, have you attached your account?"));
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
            Q_EMIT window.loginError(
                i18n("Square Enix servers refused to confirm session information. The game may be under maintenance, try the official launcher."));
        }
    });
}

void SquareLauncher::login(const LoginInformation &info, const QUrl &referer)
{
    QUrlQuery postData;
    postData.addQueryItem("_STORED_", stored);
    postData.addQueryItem("sqexid", info.username);
    postData.addQueryItem("password", info.password);
    postData.addQueryItem("otppw", info.oneTimePassword);

    QUrl url;
    url.setScheme("https");
    url.setHost(QStringLiteral("ffxiv-login.%1").arg(window.squareEnixLoginServer()));
    url.setPath("/oauth/ffxivarr/login/login.send");

    QNetworkRequest request(url);
    window.buildRequest(*info.profile, request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", referer.toEncoded());
    request.setRawHeader("Cache-Control", "no-cache");

    auto reply = window.mgr->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, [this, &info, reply] {
        auto str = QString(reply->readAll());

        QRegularExpression re(R"lit(window.external.user\("login=auth,ok,(?<launchParams>.*)\);)lit");
        QRegularExpressionMatch match = re.match(str);
        if (match.hasMatch()) {
            const auto parts = match.captured(1).split(',');

            const bool terms = parts[3] == "1";
            const bool playable = parts[9] == "1";

            if (!playable) {
                Q_EMIT window.loginError(i18n("Your account is unplayable. Check that you have the correct license, and a valid subscription."));
                return;
            }

            if (!terms) {
                Q_EMIT window.loginError(i18n("Your account is unplayable. You need to accept the terms of service from the official launcher first."));
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

            Q_EMIT window.loginError(errorStr);
        }
    });
}

void SquareLauncher::registerSession(const LoginInformation &info)
{
    QUrl url;
    url.setScheme("https");
    url.setHost(QStringLiteral("patch-gamever.%1").arg(window.squareEnixServer()));
    url.setPath(QString("/http/win32/ffxivneo_release_game/%1/%2").arg(info.profile->repositories.repositories[0].version, SID));

    auto request = QNetworkRequest(url);
    window.setSSL(request);
    request.setRawHeader("X-Hash-Check", "enabled");
    request.setRawHeader("User-Agent", "FFXIV PATCH CLIENT");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString report = QString("%1=%2").arg(info.profile->bootVersion, getBootHash(info));

    for (int i = 1; i < auth.maxExpansion + 1; i++) {
        if (i <= static_cast<int>(info.profile->repositories.repositories_count)) {
            report += QString("\nex%1\t%2").arg(QString::number(i), info.profile->repositories.repositories[i].version);
        } else {
            report += QString("\nex%1\t2012.01.01.0000.0000").arg(QString::number(i));
        }
    }

    auto reply = window.mgr->post(request, report.toUtf8());
    connect(reply, &QNetworkReply::finished, [this, &info, reply] {
        if (reply->error() == QNetworkReply::NoError) {
            if (reply->rawHeaderList().contains("X-Patch-Unique-Id")) {
                QString body = reply->readAll();

                patcher = new Patcher(window, info.profile->gamePath() + "/game", info.profile->gameData, this);
                connect(patcher, &Patcher::done, [this, &info, reply] {
                    info.profile->readGameVersion();

                    auth.SID = reply->rawHeader("X-Patch-Unique-Id");

                    window.launchGame(*info.profile, auth);
                });

                patcher->processPatchList(*window.mgr, body);
            } else {
                Q_EMIT window.loginError(i18n("Fatal error, request was successful but X-Patch-Unique-Id was not recieved."));
            }
        } else {
            if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
                Q_EMIT window.loginError(
                    i18n("SSL handshake error detected. If you are using OpenSUSE or Fedora, try running `update-crypto-policies --set LEGACY`."));
            } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 405) {
                Q_EMIT window.loginError(i18n("The game failed the anti-tamper check. Restore the game to the original state and try updating again."));
            } else {
                Q_EMIT window.loginError(i18n("Unknown error when registering the session."));
            }
        }
    });
}

QString SquareLauncher::getBootHash(const LoginInformation &info)
{
    const QList<QString> fileList = {"ffxivboot.exe", "ffxivboot64.exe", "ffxivlauncher.exe", "ffxivlauncher64.exe", "ffxivupdater.exe", "ffxivupdater64.exe"};

    QString result;
    for (int i = 0; i < fileList.count(); i++) {
        result += fileList[i] + "/" + getFileHash(info.profile->gamePath() + "/boot/" + fileList[i]);

        if (i != fileList.length() - 1)
            result += ",";
    }

    return result;
}